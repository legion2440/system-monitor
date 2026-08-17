#pragma once

#include "core/MetricProvider.hpp"
#include <memory>

namespace monesys {
std::unique_ptr<MetricProvider> createPlatformMetricProvider();
}
