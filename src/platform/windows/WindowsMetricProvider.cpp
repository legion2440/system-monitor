#include "platform/windows/WindowsMetricProvider.hpp"
namespace monesys {
Snapshot WindowsMetricProvider::poll() {
    Snapshot snapshot;
    snapshot.system.osName = "Windows";
    snapshot.error = "Windows telemetry provider is scaffolded but not implemented yet";
    snapshot.capabilities = {};
    snapshot.capabilities.cpu = false;
    snapshot.capabilities.memory = false;
    snapshot.capabilities.disk = false;
    snapshot.capabilities.network = false;
    snapshot.capabilities.processes = false;
    return snapshot;
}
}
