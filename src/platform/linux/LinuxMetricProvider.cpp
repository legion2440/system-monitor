#include "platform/linux/LinuxMetricProvider.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <thread>

namespace fs = std::filesystem;

namespace monesys {
namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n\"");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n\"");
    return value.substr(first, last - first + 1);
}

std::string readTextFile(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) return {};
    std::ostringstream out;
    out << stream.rdbuf();
    return out.str();
}

std::string readFirstLine(const fs::path& path) {
    std::ifstream stream(path);
    std::string line;
    std::getline(stream, line);
    return trim(line);
}

bool isPidDirectory(const fs::directory_entry& entry) {
    std::error_code ec;
    if (!entry.is_directory(ec) || ec) return false;
    const auto name = entry.path().filename().string();
    return !name.empty() && std::all_of(name.begin(), name.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

std::string processStateName(char state) {
    switch (state) {
    case 'R': return "Running";
    case 'S': return "Sleeping";
    case 'D': return "Uninterruptible";
    case 'Z': return "Zombie";
    case 'T': return "Stopped";
    case 't': return "Traced";
    case 'I': return "Idle";
    case 'X': case 'x': return "Dead";
    default: return std::string(1, state);
    }
}

std::string userNameFromUid(uid_t uid) {
    if (const passwd* pw = ::getpwuid(uid); pw != nullptr && pw->pw_name != nullptr) return pw->pw_name;
    return std::to_string(uid);
}

std::uint64_t parseUnsigned(const std::string& value) { try { return std::stoull(value); } catch (...) { return 0; } }
std::int64_t parseSigned(const std::string& value) { try { return std::stoll(value); } catch (...) { return 0; } }
double parseDouble(const std::string& value) { try { return std::stod(value); } catch (...) { return 0.0; } }

std::unordered_map<std::string, std::uint64_t> readMemInfoValues() {
    std::unordered_map<std::string, std::uint64_t> values;
    std::ifstream stream("/proc/meminfo");
    std::string line;
    while (std::getline(stream, line)) {
        std::istringstream row(line);
        std::string key, unit;
        std::uint64_t value{};
        if (!(row >> key >> value)) continue;
        if (!key.empty() && key.back() == ':') key.pop_back();
        row >> unit;
        values[key] = value;
    }
    return values;
}

std::uint64_t memInfoBytes(const std::unordered_map<std::string, std::uint64_t>& values, const std::string& key) {
    const auto it = values.find(key);
    return it == values.end() ? 0 : it->second * 1024ULL;
}

bool rootHasPrefix(const fs::path& root, std::string_view prefix) {
    std::error_code ec;
    if (!fs::exists(root, ec)) return false;
    for (const auto& entry : fs::directory_iterator(root, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) break;
        if (entry.path().filename().string().rfind(prefix, 0) == 0) return true;
    }
    return false;
}

std::string linuxDisplayName(const std::string& distribution) {
    if (distribution.empty()) return "Linux";
    std::string lower = distribution;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lower.find("linux") != std::string::npos) return distribution;
    return "Linux · " + distribution;
}

} // namespace

LinuxMetricProvider::LinuxMetricProvider() {
    clockTicksPerSecond_ = std::max<long>(1, ::sysconf(_SC_CLK_TCK));
    pageSize_ = std::max<long>(1, ::sysconf(_SC_PAGESIZE));
    logicalCpuCount_ = std::max(1u, std::thread::hardware_concurrency());
    staticSystem_.osName = readOsName();
    staticSystem_.userName = readUserName();
    staticSystem_.hostname = readHostname();
    staticSystem_.cpuModel = readCpuModel();
    detectCapabilities();
}

Snapshot LinuxMetricProvider::poll() {
    Snapshot snapshot;
    snapshot.timestamp = Snapshot::Clock::now();
    snapshot.capabilities = capabilities_;
    snapshot.system = staticSystem_;
    const auto currentCpu = readCpuTimes();
    std::uint64_t totalCpuDelta{};
    snapshot.cpu = buildCpuStats(currentCpu, totalCpuDelta);
    snapshot.memory = readMemoryAndDisk();
    snapshot.processes = readProcesses(totalCpuDelta, snapshot.system.tasks);
    snapshot.network = readNetwork();
    snapshot.sensors = readSensors();
    snapshot.fan = readFan();
    snapshot.capabilities.thermal = !snapshot.sensors.empty();
    snapshot.capabilities.fan = snapshot.fan.available;
    capabilities_.thermal = snapshot.capabilities.thermal;
    capabilities_.fan = snapshot.capabilities.fan;
    return snapshot;
}

std::string LinuxMetricProvider::readOsName() {
    std::ifstream stream("/etc/os-release");
    std::string line, pretty, name;
    while (std::getline(stream, line)) {
        const auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        const auto key = line.substr(0, pos);
        const auto value = trim(line.substr(pos + 1));
        if (key == "PRETTY_NAME") pretty = value;
        else if (key == "NAME") name = value;
    }
    if (!pretty.empty()) return linuxDisplayName(pretty);
    if (!name.empty()) return linuxDisplayName(name);
    return "Linux";
}

std::string LinuxMetricProvider::readUserName() {
    if (const char* login = ::getlogin(); login != nullptr && *login != '\0') return login;
    if (const char* user = std::getenv("USER"); user != nullptr && *user != '\0') return user;
    return userNameFromUid(::getuid());
}

std::string LinuxMetricProvider::readHostname() {
    std::array<char, 256> buffer{};
    if (::gethostname(buffer.data(), buffer.size() - 1) == 0) return buffer.data();
    return {};
}

std::string LinuxMetricProvider::readCpuModel() {
    std::ifstream stream("/proc/cpuinfo");
    std::string line;
    while (std::getline(stream, line)) {
        const auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        const auto key = trim(line.substr(0, pos));
        if (key == "model name" || key == "Hardware" || key == "Processor") {
            const auto model = trim(line.substr(pos + 1));
            if (!model.empty()) return model;
        }
    }
    return "Unknown CPU";
}

double LinuxMetricProvider::readCpuFrequencyMHz() {
    std::ifstream stream("/proc/cpuinfo");
    std::string line;
    double sum{};
    std::size_t count{};
    while (std::getline(stream, line)) {
        const auto pos = line.find(':');
        if (pos != std::string::npos && trim(line.substr(0, pos)) == "cpu MHz") {
            sum += parseDouble(trim(line.substr(pos + 1)));
            ++count;
        }
    }
    return count == 0 ? 0.0 : sum / static_cast<double>(count);
}

LinuxMetricProvider::CpuTimes LinuxMetricProvider::readCpuTimes() {
    CpuTimes result;
    std::ifstream stream("/proc/stat");
    std::string line;
    while (std::getline(stream, line)) {
        if (line.rfind("cpu", 0) != 0) break;
        std::istringstream row(line);
        std::string label;
        row >> label;
        std::vector<std::uint64_t> values;
        std::uint64_t value{};
        while (row >> value) values.push_back(value);
        if (values.size() < 4) continue;
        std::uint64_t total{};
        for (const auto tick : values) total += tick;
        const std::uint64_t idle = values[3] + (values.size() > 4 ? values[4] : 0);
        if (label == "cpu") { result.total = total; result.idle = idle; }
        else result.cores.emplace_back(total, idle);
    }
    return result;
}

CpuStats LinuxMetricProvider::buildCpuStats(const CpuTimes& current, std::uint64_t& totalDelta) {
    CpuStats stats;
    stats.frequencyMHz = readCpuFrequencyMHz();
    stats.coreUsagePercent.resize(current.cores.size(), 0.0);
    if (!previousCpu_) { previousCpu_ = current; return stats; }
    const auto& previous = *previousCpu_;
    totalDelta = current.total >= previous.total ? current.total - previous.total : 0;
    const auto idleDelta = current.idle >= previous.idle ? current.idle - previous.idle : 0;
    if (totalDelta > 0) stats.usagePercent = 100.0 * static_cast<double>(totalDelta - std::min(totalDelta, idleDelta)) / static_cast<double>(totalDelta);
    const auto count = std::min(current.cores.size(), previous.cores.size());
    for (std::size_t i = 0; i < count; ++i) {
        const auto delta = current.cores[i].first >= previous.cores[i].first ? current.cores[i].first - previous.cores[i].first : 0;
        const auto idle = current.cores[i].second >= previous.cores[i].second ? current.cores[i].second - previous.cores[i].second : 0;
        if (delta > 0) stats.coreUsagePercent[i] = 100.0 * static_cast<double>(delta - std::min(delta, idle)) / static_cast<double>(delta);
    }
    previousCpu_ = current;
    return stats;
}

MemoryStats LinuxMetricProvider::readMemoryAndDisk() {
    MemoryStats stats;
    const auto mem = readMemInfoValues();
    stats.ramTotalBytes = memInfoBytes(mem, "MemTotal");
    const auto available = memInfoBytes(mem, "MemAvailable");
    stats.ramUsedBytes = stats.ramTotalBytes >= available ? stats.ramTotalBytes - available : 0;
    stats.swapTotalBytes = memInfoBytes(mem, "SwapTotal");
    const auto swapFree = memInfoBytes(mem, "SwapFree");
    stats.swapUsedBytes = stats.swapTotalBytes >= swapFree ? stats.swapTotalBytes - swapFree : 0;
    struct statvfs fsStats {};
    if (::statvfs("/", &fsStats) == 0) {
        stats.diskTotalBytes = static_cast<std::uint64_t>(fsStats.f_blocks) * fsStats.f_frsize;
        const auto freeBytes = static_cast<std::uint64_t>(fsStats.f_bfree) * fsStats.f_frsize;
        stats.diskUsedBytes = stats.diskTotalBytes >= freeBytes ? stats.diskTotalBytes - freeBytes : 0;
    }
    return stats;
}

std::vector<ProcessInfo> LinuxMetricProvider::readProcesses(std::uint64_t totalCpuDelta, TaskStats& tasks) {
    std::vector<ProcessInfo> processes;
    std::unordered_map<std::int64_t, std::uint64_t> nextTicks;
    const auto ramTotal = memInfoBytes(readMemInfoValues(), "MemTotal");
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator("/proc", fs::directory_options::skip_permission_denied, ec)) {
        if (ec || !isPidDirectory(entry)) continue;
        const auto pid = parseSigned(entry.path().filename().string());
        const auto statLine = readFirstLine(entry.path() / "stat");
        const auto open = statLine.find('('), close = statLine.rfind(')');
        if (open == std::string::npos || close == std::string::npos || close <= open || close + 2 >= statLine.size()) continue;
        ProcessInfo process;
        process.pid = pid;
        process.name = statLine.substr(open + 1, close - open - 1);
        std::istringstream fieldsStream(statLine.substr(close + 2));
        std::vector<std::string> fields;
        std::string field;
        while (fieldsStream >> field) fields.push_back(field);
        if (fields.size() < 22) continue;
        const char state = fields[0].empty() ? '?' : fields[0][0];
        process.stateCode = std::string(1, state);
        process.state = processStateName(state);
        process.parentPid = parseSigned(fields[1]);
        const auto processTicks = parseUnsigned(fields[11]) + parseUnsigned(fields[12]);
        process.threads = static_cast<std::uint32_t>(parseUnsigned(fields[17]));
        process.virtualBytes = parseUnsigned(fields[20]);
        process.rssBytes = static_cast<std::uint64_t>(std::max<std::int64_t>(0, parseSigned(fields[21]))) * static_cast<std::uint64_t>(pageSize_);
        process.memoryPercent = ramTotal == 0 ? 0.0 : 100.0 * static_cast<double>(process.rssBytes) / static_cast<double>(ramTotal);
        if (const auto previous = previousProcessTicks_.find(pid); previous != previousProcessTicks_.end() && totalCpuDelta > 0) {
            const auto delta = processTicks >= previous->second ? processTicks - previous->second : 0;
            process.cpuPercent = 100.0 * static_cast<double>(delta) * static_cast<double>(logicalCpuCount_) / static_cast<double>(totalCpuDelta);
        }
        nextTicks[pid] = processTicks;
        struct stat fileStats {};
        const auto procPath = entry.path().string();
        if (::stat(procPath.c_str(), &fileStats) == 0) process.user = userNameFromUid(fileStats.st_uid);
        process.commandLine = readTextFile(entry.path() / "cmdline");
        std::replace(process.commandLine.begin(), process.commandLine.end(), '\0', ' ');
        process.commandLine = trim(process.commandLine);
        process.cgroup = trim(readTextFile(entry.path() / "cgroup"));
        std::ifstream io(entry.path() / "io");
        std::string key; std::uint64_t value{};
        while (io >> key >> value) {
            if (key == "read_bytes:") process.readBytes = value;
            else if (key == "write_bytes:") process.writeBytes = value;
        }
        ++tasks.total;
        switch (state) {
        case 'R': ++tasks.running; break;
        case 'S': case 'I': ++tasks.sleeping; break;
        case 'D': ++tasks.uninterruptible; break;
        case 'Z': ++tasks.zombie; break;
        case 'T': ++tasks.stopped; break;
        case 't': ++tasks.traced; break;
        default: ++tasks.other; break;
        }
        processes.push_back(std::move(process));
    }
    previousProcessTicks_.swap(nextTicks);
    std::sort(processes.begin(), processes.end(), [](const ProcessInfo& left, const ProcessInfo& right) {
        if (std::abs(left.cpuPercent - right.cpuPercent) > 0.001) return left.cpuPercent > right.cpuPercent;
        return left.pid < right.pid;
    });
    return processes;
}

std::unordered_map<std::string, std::string> LinuxMetricProvider::readIpv4Addresses() {
    std::unordered_map<std::string, std::string> result;
    ifaddrs* addresses = nullptr;
    if (::getifaddrs(&addresses) != 0) return result;
    for (auto* current = addresses; current != nullptr; current = current->ifa_next) {
        if (current->ifa_addr == nullptr || current->ifa_addr->sa_family != AF_INET) continue;
        std::array<char, INET_ADDRSTRLEN> buffer{};
        const auto* ipv4 = reinterpret_cast<sockaddr_in*>(current->ifa_addr);
        if (::inet_ntop(AF_INET, &ipv4->sin_addr, buffer.data(), buffer.size()) != nullptr) result[current->ifa_name] = buffer.data();
    }
    ::freeifaddrs(addresses);
    return result;
}

std::vector<NetworkInterfaceStats> LinuxMetricProvider::readNetwork() {
    std::vector<NetworkInterfaceStats> interfaces;
    const auto ipv4 = readIpv4Addresses();
    const auto now = std::chrono::steady_clock::now();
    const auto seconds = previousNetworkTime_ == std::chrono::steady_clock::time_point{} ? 0.0 : std::chrono::duration<double>(now - previousNetworkTime_).count();
    std::unordered_map<std::string, NetworkPrevious> next;
    std::ifstream stream("/proc/net/dev");
    std::string line;
    std::getline(stream, line); std::getline(stream, line);
    while (std::getline(stream, line)) {
        const auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        NetworkInterfaceStats item;
        item.name = trim(line.substr(0, colon));
        if (const auto address = ipv4.find(item.name); address != ipv4.end()) item.ipv4 = address->second;
        std::istringstream values(line.substr(colon + 1));
        values >> item.rx.bytes >> item.rx.packets >> item.rx.errors >> item.rx.dropped >> item.rx.fifo >> item.rx.frameOrCollisions >> item.rx.compressed >> item.rx.multicastOrCarrier >> item.tx.bytes >> item.tx.packets >> item.tx.errors >> item.tx.dropped >> item.tx.fifo >> item.tx.frameOrCollisions >> item.tx.multicastOrCarrier >> item.tx.compressed;
        if (const auto previous = previousNetwork_.find(item.name); previous != previousNetwork_.end() && seconds > 0.0) {
            if (item.rx.bytes >= previous->second.rx) item.rxBytesPerSecond = static_cast<double>(item.rx.bytes - previous->second.rx) / seconds;
            if (item.tx.bytes >= previous->second.tx) item.txBytesPerSecond = static_cast<double>(item.tx.bytes - previous->second.tx) / seconds;
        }
        next[item.name] = {item.rx.bytes, item.tx.bytes};
        interfaces.push_back(std::move(item));
    }
    previousNetwork_ = std::move(next);
    previousNetworkTime_ = now;
    return interfaces;
}

std::vector<SensorInfo> LinuxMetricProvider::readSensors() {
    std::vector<SensorInfo> sensors;
    std::error_code ec;
    const fs::path thermalRoot("/sys/class/thermal");
    if (fs::exists(thermalRoot, ec)) {
        for (const auto& entry : fs::directory_iterator(thermalRoot, fs::directory_options::skip_permission_denied, ec)) {
            if (ec || entry.path().filename().string().rfind("thermal_zone", 0) != 0) continue;
            const auto tempText = readFirstLine(entry.path() / "temp");
            if (tempText.empty()) continue;
            const auto raw = parseDouble(tempText);
            SensorInfo sensor;
            sensor.name = readFirstLine(entry.path() / "type");
            if (sensor.name.empty()) sensor.name = entry.path().filename().string();
            sensor.source = (entry.path() / "temp").string();
            sensor.unit = "°C";
            sensor.value = std::abs(raw) > 1000.0 ? raw / 1000.0 : raw;
            sensors.push_back(std::move(sensor));
        }
    }
    const fs::path hwmonRoot("/sys/class/hwmon");
    ec.clear();
    if (fs::exists(hwmonRoot, ec)) {
        for (const auto& hwmon : fs::directory_iterator(hwmonRoot, fs::directory_options::skip_permission_denied, ec)) {
            if (ec) break;
            const auto chip = readFirstLine(hwmon.path() / "name");
            for (const auto& file : fs::directory_iterator(hwmon.path(), fs::directory_options::skip_permission_denied, ec)) {
                const auto name = file.path().filename().string();
                if (name.rfind("temp", 0) != 0 || name.find("_input") == std::string::npos) continue;
                const auto rawText = readFirstLine(file.path());
                if (rawText.empty()) continue;
                const auto prefix = name.substr(0, name.find("_input"));
                auto label = readFirstLine(hwmon.path() / (prefix + "_label"));
                if (label.empty()) label = chip.empty() ? prefix : chip + " " + prefix;
                const auto raw = parseDouble(rawText);
                sensors.push_back({label, file.path().string(), "°C", std::abs(raw) > 1000.0 ? raw / 1000.0 : raw});
            }
        }
    }
    return sensors;
}

FanInfo LinuxMetricProvider::readFan() {
    FanInfo fan;
    std::error_code ec;
    const fs::path root("/sys/class/hwmon");
    if (!fs::exists(root, ec)) return fan;
    for (const auto& hwmon : fs::directory_iterator(root, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) break;
        const auto chip = readFirstLine(hwmon.path() / "name");
        for (const auto& file : fs::directory_iterator(hwmon.path(), fs::directory_options::skip_permission_denied, ec)) {
            const auto filename = file.path().filename().string();
            if (filename.rfind("fan", 0) != 0 || filename.find("_input") == std::string::npos) continue;
            const auto rawText = readFirstLine(file.path());
            if (rawText.empty()) continue;
            const auto prefix = filename.substr(0, filename.find("_input"));
            fan.available = true;
            fan.rpm = parseDouble(rawText);
            fan.active = fan.rpm > 0.0;
            fan.name = readFirstLine(hwmon.path() / (prefix + "_label"));
            if (fan.name.empty()) fan.name = chip.empty() ? prefix : chip + " " + prefix;
            fan.source = file.path().string();
            std::string index;
            for (char c : prefix) if (std::isdigit(static_cast<unsigned char>(c)) != 0) index.push_back(c);
            if (!index.empty()) {
                const auto pwmText = readFirstLine(hwmon.path() / ("pwm" + index));
                if (!pwmText.empty()) fan.levelPercent = std::clamp(parseDouble(pwmText) / 255.0 * 100.0, 0.0, 100.0);
            }
            return fan;
        }
    }
    return fan;
}

void LinuxMetricProvider::detectCapabilities() {
    capabilities_.cpu = fs::exists("/proc/stat") && fs::exists("/proc/cpuinfo");
    capabilities_.memory = fs::exists("/proc/meminfo");
    capabilities_.disk = true;
    capabilities_.network = fs::exists("/proc/net/dev");
    capabilities_.processes = fs::exists("/proc");
    capabilities_.thermal = rootHasPrefix("/sys/class/thermal", "thermal_zone") || rootHasPrefix("/sys/class/hwmon", "hwmon");
    capabilities_.fan = rootHasPrefix("/sys/class/hwmon", "hwmon");
    capabilities_.energy = fs::exists("/sys/class/powercap");
    capabilities_.services = fs::exists("/run/systemd/system");
    capabilities_.logs = fs::exists("/run/log/journal") || fs::exists("/var/log/journal");
}

} // namespace monesys
