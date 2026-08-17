#include "core/TimedHistory.hpp"
#include <cassert>
#include <chrono>

int main() {
    using namespace std::chrono_literals;
    using History = monesys::TimedHistory<double>;
    History history(1000ms);
    const auto base = History::Clock::now();
    history.push(10.0, base);
    history.push(20.0, base + 500ms);
    history.push(30.0, base + 1500ms);
    const auto values = history.values();
    assert(values.size() == 2);
    assert(values[0] == 20.0);
    assert(values[1] == 30.0);
    history.clear();
    assert(history.size() == 0);
    return 0;
}
