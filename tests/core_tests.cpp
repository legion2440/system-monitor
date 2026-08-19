#include "core/TimedHistory.hpp"

#include <chrono>
#include <cstdio>

namespace {
int failures = 0;

void check(bool condition, const char* what) {
    if (!condition) {
        ++failures;
        std::fprintf(stderr, "FAIL: %s\n", what);
    }
}
} // namespace

int main() {
    using namespace std::chrono_literals;
    using History = monesys::TimedHistory<double>;

    History history(1000ms);
    const auto base = History::Clock::now();
    history.push(10.0, base);
    history.push(20.0, base + 500ms);
    history.push(30.0, base + 1500ms);

    const auto values = history.values();
    check(values.size() == 2, "window trims samples older than the cutoff");
    if (values.size() == 2) {
        check(values[0] == 20.0, "oldest surviving sample is kept");
        check(values[1] == 30.0, "newest sample is kept");
    }

    history.clear();
    check(history.size() == 0, "clear empties the buffer");

    History wide(10s);
    wide.push(1.0, base);
    wide.push(2.0, base + 9s);
    check(wide.size() == 2, "a wider window keeps both samples");

    if (failures == 0) std::puts("core_tests: OK");
    return failures == 0 ? 0 : 1;
}
