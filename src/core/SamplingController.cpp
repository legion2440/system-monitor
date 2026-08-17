#include "core/SamplingController.hpp"

#include <algorithm>
#include <exception>
#include <string>

namespace monesys {

SamplingController::SamplingController(std::unique_ptr<MetricProvider> provider, Callback callback)
    : provider_(std::move(provider)), callback_(std::move(callback)) {}

SamplingController::~SamplingController() { stop(); }

void SamplingController::start() {
    if (worker_.joinable()) return;
    worker_ = std::jthread([this](std::stop_token token) { run(token); });
}

void SamplingController::stop() {
    if (!worker_.joinable()) return;
    worker_.request_stop();
    worker_.join();
}

void SamplingController::setInterval(std::chrono::milliseconds interval) {
    intervalMs_.store(std::clamp<int>(static_cast<int>(interval.count()), 100, 60'000));
}

void SamplingController::setPaused(bool paused) noexcept { paused_.store(paused); }

void SamplingController::run(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        if (paused_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        const auto started = std::chrono::steady_clock::now();
        Snapshot snapshot;
        try { snapshot = provider_->poll(); }
        catch (const std::exception& ex) { snapshot.error = ex.what(); }
        catch (...) { snapshot.error = "unknown metric provider failure"; }
        if (callback_) callback_(std::move(snapshot));
        const auto interval = std::chrono::milliseconds(intervalMs_.load());
        const auto elapsed = std::chrono::steady_clock::now() - started;
        if (elapsed < interval) {
            auto remaining = interval - std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
            while (remaining.count() > 0 && !stopToken.stop_requested()) {
                const auto slice = std::min(remaining, std::chrono::milliseconds(50));
                std::this_thread::sleep_for(slice);
                remaining -= slice;
            }
        }
    }
}

} // namespace monesys
