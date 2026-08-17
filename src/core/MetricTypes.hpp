#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace monesys {

struct Capabilities {
    bool cpu{true};
    bool memory{true};
    bool disk{true};
    bool network{true};
    bool processes{true};
    bool thermal{false};
    bool fan{false};
    bool gpu{false};
    bool energy{false};
    bool services{false};
    bool logs{false};
};

struct TaskStats {
    std::uint64_t total{};
    std::uint64_t running{};
    std::uint64_t sleeping{};
    std::uint64_t uninterruptible{};
    std::uint64_t zombie{};
    std::uint64_t stopped{};
    std::uint64_t traced{};
    std::uint64_t other{};
};

struct SystemInfo {
    std::string osName;
    std::string userName;
    std::string hostname;
    std::string cpuModel;
    TaskStats tasks;
};

struct CpuStats {
    double usagePercent{};
    double frequencyMHz{};
    std::vector<double> coreUsagePercent;
};

struct MemoryStats {
    std::uint64_t ramTotalBytes{};
    std::uint64_t ramUsedBytes{};
    std::uint64_t swapTotalBytes{};
    std::uint64_t swapUsedBytes{};
    std::uint64_t diskTotalBytes{};
    std::uint64_t diskUsedBytes{};
};

struct NetworkCounters {
    std::uint64_t bytes{};
    std::uint64_t packets{};
    std::uint64_t errors{};
    std::uint64_t dropped{};
    std::uint64_t fifo{};
    std::uint64_t frameOrCollisions{};
    std::uint64_t compressed{};
    std::uint64_t multicastOrCarrier{};
};

struct NetworkInterfaceStats {
    std::string name;
    std::string ipv4;
    NetworkCounters rx;
    NetworkCounters tx;
    double rxBytesPerSecond{};
    double txBytesPerSecond{};
};

struct ProcessInfo {
    std::int64_t pid{};
    std::int64_t parentPid{};
    std::string name;
    std::string commandLine;
    std::string state;
    std::string user;
    std::string cgroup;
    double cpuPercent{};
    double memoryPercent{};
    std::uint64_t rssBytes{};
    std::uint64_t virtualBytes{};
    std::uint64_t readBytes{};
    std::uint64_t writeBytes{};
    std::uint32_t threads{};
};

struct SensorInfo {
    std::string name;
    std::string source;
    std::string unit;
    double value{};
};

struct FanInfo {
    bool available{false};
    bool active{false};
    std::string name;
    std::string source;
    double rpm{};
    double levelPercent{-1.0};
};

struct Snapshot {
    using Clock = std::chrono::steady_clock;
    Clock::time_point timestamp{Clock::now()};
    Capabilities capabilities;
    SystemInfo system;
    CpuStats cpu;
    MemoryStats memory;
    std::vector<NetworkInterfaceStats> network;
    std::vector<ProcessInfo> processes;
    std::vector<SensorInfo> sensors;
    FanInfo fan;
    std::string error;
};

} // namespace monesys
