#pragma once

#include "core/MetricProvider.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace monesys {

class LinuxMetricProvider final : public MetricProvider {
public:
    LinuxMetricProvider();
    [[nodiscard]] std::string_view platformName() const noexcept override { return "linux"; }
    [[nodiscard]] Capabilities capabilities() const noexcept override { return capabilities_; }
    Snapshot poll() override;

private:
    struct CpuTimes {
        std::uint64_t total{};
        std::uint64_t idle{};
        std::vector<std::pair<std::uint64_t, std::uint64_t>> cores;
    };
    struct NetworkPrevious { std::uint64_t rx{}; std::uint64_t tx{}; };
    struct NetworkBaseline { std::uint64_t rx{}; std::uint64_t tx{}; };

    static std::string readOsName();
    static std::string readUserName();
    static std::string readHostname();
    static std::string readCpuModel();
    static double readCpuFrequencyMHz();
    static CpuTimes readCpuTimes();
    static MemoryStats readMemoryAndDisk(const std::unordered_map<std::string, std::uint64_t>& meminfo);
    static std::unordered_map<std::string, std::string> readIpv4Addresses();
    static std::vector<SensorInfo> readSensors();
    static FanInfo readFan();

    CpuStats buildCpuStats(const CpuTimes& current, std::uint64_t& totalDelta);
    std::vector<ProcessInfo> readProcesses(std::uint64_t totalCpuDelta, std::uint64_t ramTotalBytes, TaskStats& tasks);
    std::vector<NetworkInterfaceStats> readNetwork();
    void detectCapabilities();

    Capabilities capabilities_;
    SystemInfo staticSystem_;
    std::optional<CpuTimes> previousCpu_;
    std::unordered_map<std::int64_t, std::uint64_t> previousProcessTicks_;
    std::unordered_map<std::string, NetworkPrevious> previousNetwork_;
    std::unordered_map<std::string, NetworkBaseline> networkBaseline_;
    std::chrono::steady_clock::time_point previousNetworkTime_{};
    long clockTicksPerSecond_{100};
    long pageSize_{4096};
    unsigned logicalCpuCount_{1};
};

} // namespace monesys
