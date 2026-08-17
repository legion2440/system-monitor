#pragma once
#include "core/MetricProvider.hpp"
namespace monesys {
class WindowsMetricProvider final : public MetricProvider {
public:
    [[nodiscard]] std::string_view platformName() const noexcept override { return "windows"; }
    [[nodiscard]] Capabilities capabilities() const noexcept override { return {}; }
    Snapshot poll() override;
};
}
