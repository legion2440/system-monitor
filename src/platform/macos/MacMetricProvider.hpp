#pragma once
#include "core/MetricProvider.hpp"
namespace monesys {
class MacMetricProvider final : public MetricProvider {
public:
    [[nodiscard]] std::string_view platformName() const noexcept override { return "macos"; }
    [[nodiscard]] Capabilities capabilities() const noexcept override { return {}; }
    Snapshot poll() override;
};
}
