#pragma once

#include <chrono>
#include <deque>
#include <utility>
#include <vector>

namespace monesys {

template <typename T>
class TimedHistory {
public:
    using Clock = std::chrono::steady_clock;
    using Sample = std::pair<Clock::time_point, T>;

    explicit TimedHistory(std::chrono::milliseconds window = std::chrono::seconds(60)) : window_(window) {}

    void setWindow(std::chrono::milliseconds window) {
        window_ = window;
        trim(Clock::now());
    }

    void push(T value, Clock::time_point now = Clock::now()) {
        samples_.emplace_back(now, std::move(value));
        trim(now);
    }

    void clear() { samples_.clear(); }

    [[nodiscard]] std::vector<T> values() const {
        std::vector<T> out;
        out.reserve(samples_.size());
        for (const auto& sample : samples_) out.push_back(sample.second);
        return out;
    }

    [[nodiscard]] std::size_t size() const noexcept { return samples_.size(); }

private:
    void trim(Clock::time_point now) {
        const auto cutoff = now - window_;
        while (!samples_.empty() && samples_.front().first < cutoff) samples_.pop_front();
    }

    std::chrono::milliseconds window_;
    std::deque<Sample> samples_;
};

} // namespace monesys
