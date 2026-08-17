#include "platform/ProviderFactory.hpp"

#if defined(__linux__)
#include "platform/linux/LinuxMetricProvider.hpp"
#elif defined(_WIN32)
#include "platform/windows/WindowsMetricProvider.hpp"
#elif defined(__APPLE__)
#include "platform/macos/MacMetricProvider.hpp"
#endif

namespace monesys {
std::unique_ptr<MetricProvider> createPlatformMetricProvider() {
#if defined(__linux__)
    return std::make_unique<LinuxMetricProvider>();
#elif defined(_WIN32)
    return std::make_unique<WindowsMetricProvider>();
#elif defined(__APPLE__)
    return std::make_unique<MacMetricProvider>();
#else
#error Unsupported MoneSys platform
#endif
}
}
