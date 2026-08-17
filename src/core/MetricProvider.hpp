#pragma once

#include "core/MetricTypes.hpp"

#include <string_view>

namespace monesys {

class MetricProvider {
public:
    virtual ~MetricProvider() = default;
    [[nodiscard]] virtual std::string_view platformName() const noexcept = 0;
    [[nodiscard]] virtual Capabilities capabilities() const noexcept = 0;
    virtual Snapshot poll() = 0;
};

} // namespace monesys
