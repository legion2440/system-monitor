#include "platform/macos/MacMetricProvider.hpp"
namespace monesys {
Snapshot MacMetricProvider::poll() {
    Snapshot snapshot;
    snapshot.system.osName = "macOS";
    snapshot.error = "macOS telemetry provider is scaffolded but not implemented yet";
    snapshot.capabilities = {};
    snapshot.capabilities.cpu = false;
    snapshot.capabilities.memory = false;
    snapshot.capabilities.disk = false;
    snapshot.capabilities.network = false;
    snapshot.capabilities.processes = false;
    return snapshot;
}
}
