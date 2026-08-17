#pragma once

#include "core/MetricProvider.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

namespace monesys {

class SamplingController {
public:
    using Callback = std::function<void(Snapshot)>;

    SamplingController(std::unique_ptr<MetricProvider> provider, Callback callback);
    ~SamplingController();
    SamplingController(const SamplingController&) = delete;
    SamplingController& operator=(const SamplingController&) = delete;

    void start();
    void stop();
    void setInterval(std::chrono::milliseconds interval);
    void setPaused(bool paused) noexcept;
    [[nodiscard]] bool paused() const noexcept { return paused_.load(); }
    [[nodiscard]] int intervalMs() const noexcept { return intervalMs_.load(); }

private:
    void run(std::stop_token stopToken);
    std::unique_ptr<MetricProvider> provider_;
    Callback callback_;
    std::jthread worker_;
    std::atomic<int> intervalMs_{1000};
    std::atomic<bool> paused_{false};
};

} // namespace monesys
