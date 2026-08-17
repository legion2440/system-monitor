# MoneSys

A modern C++20 cross-platform system telemetry and process monitor. MoneSys uses a native Qt 6 Quick/QML interface, a platform-neutral metrics core, and separate operating-system providers so Linux, Windows and macOS can share the same UI and metric semantics without coupling telemetry code to Qt.

The current milestone implements the Linux backend required by the 01-edu `system-monitor` assignment while keeping Windows and macOS provider boundaries ready for native backends.

· [Русская версия](README_RU.md)

## 📋 TOC

- [🚀 Quick start](#-quick-start)
- [📝 About](#-about)
- [🏗️ Architecture](#️-architecture)
- [📊 Linux telemetry](#-linux-telemetry)
- [🖥️ Interface](#️-interface)
- [⚙️ Sampling and graphs](#️-sampling-and-graphs)
- [🌐 Cross-platform design](#-cross-platform-design)
- [🧪 Tests and verification](#-tests-and-verification)
- [📋 01-edu audit coverage](#-01-edu-audit-coverage)
- [📁 Project structure](#-project-structure)
- [⚠️ Notes](#️-notes)
- [🧑‍💻 Author](#-author)

## 🚀 Quick start

### Requirements

- Linux for the current complete telemetry backend
- C++20 compiler
- CMake 3.21 or newer
- Qt 6.5 or newer with Qt Quick and Qt Quick Controls 2
- Ninja is recommended but not required

Install the Qt 6 development stack using your distribution packages or the official Qt installer. Exact package names differ between distributions.

### Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run

```bash
./build/monesys
```

By default the build also creates an identical `monitor` executable for the historical 01-edu audit step that asks the evaluator to find the monitor process itself:

```bash
./build/monitor
```

Disable the alias with:

```bash
cmake -S . -B build -DMONESYS_BUILD_AUDIT_ALIAS=OFF
```

## 📝 About

MoneSys is not a wrapper around `top`, `free`, `df`, `ifconfig` or other command-line tools. The Linux provider reads kernel and system interfaces directly and exposes normalized C++ metric structures to the rest of the application.

The project deliberately replaces the assignment's old Dear ImGui presentation with a native Qt Quick/QML product UI while preserving the telemetry behavior the audit checks. Under the interface it remains a C++ system-monitoring project.

The first milestone includes:

- system identity and process-state counts;
- whole-machine and per-core CPU telemetry;
- RAM, SWAP and root filesystem usage;
- process CPU/RAM/I/O/state/user information;
- process filtering and persistent multi-selection;
- IPv4 plus complete RX/TX kernel counters;
- live network rates and adaptive byte units;
- thermal sensors and hwmon fan telemetry when available;
- 60-second metric history;
- independent polling cadence, graph FPS and Y-scale controls.

GPU, Energy, Services and Logs already have explicit UI/provider slots but are not claimed as implemented telemetry in the current milestone.

## 🏗️ Architecture

```text
Qt Quick / QML
      ↓
Qt bridge + models
      ↓
normalized C++20 metric types
      ↓
MetricProvider
  ├── LinuxMetricProvider
  ├── WindowsMetricProvider
  └── MacMetricProvider
```

`src/core/` has no Qt dependency. Platform providers return plain C++ snapshots. `src/qt/` performs the conversion into Qt properties and persistent models consumed by QML.

This means a Windows or macOS telemetry backend can be added without rewriting the UI, history buffers, process presentation or normalized metric contracts.

More detail: [`docs/architecture.md`](docs/architecture.md).

## 📊 Linux telemetry

### System and CPU

MoneSys reads `/etc/os-release`, `gethostname()`, `/proc/cpuinfo`, `/proc/stat` and `/proc/<pid>/stat` directly.

System CPU is normalized to `0..100%` for the complete machine. Process CPU uses the common task-manager convention `100% = one logical CPU`, so a multithreaded process may exceed `100%`.

### Memory and disk

- RAM/SWAP — `/proc/meminfo`;
- root filesystem — `statvfs("/")`.

Linux RAM used is calculated as `MemTotal - MemAvailable`, avoiding the misleading interpretation that all page cache is permanently occupied memory.

### Processes

The process model collects PID/PPID, name, command line, state, CPU percentage, memory percentage, RSS, virtual memory, read/write bytes, thread count, user and cgroup data when readable.

Selection is keyed by PID rather than row position, so filtering and normal refreshes do not silently move the selection to another process. Multiple rows can be selected.

### Network

MoneSys combines `getifaddrs()` for IPv4 with `/proc/net/dev` for cumulative RX/TX counters. Rates are calculated from timestamped counter deltas.

The raw tables preserve the audit fields for Receive and Transmit. User-facing values use adaptive B/KB/MB/GB units. The dedicated audit visualization uses a `0..2 GiB` progress range as requested by the subject.

### Thermal and fan

MoneSys uses generic Linux interfaces instead of depending on the old ThinkPad-specific `/proc/acpi/ibm/thermal` path:

- `/sys/class/thermal/thermal_zone*/temp`;
- `/sys/class/hwmon/hwmon*/temp*_input`;
- hwmon `fan*_input`;
- optional `pwm*` for fan level.

If hardware does not expose a sensor or fan to the kernel, the UI reports it as unavailable instead of inventing a zero value.

## 🖥️ Interface

The Qt Quick/QML UI follows the MoneSys high-fidelity desktop design rather than the original assignment screenshot.

Current screens include:

- **Overview** — CPU, memory, system tasks, disk, network, sensors and top processes;
- **CPU** — CPU/Fan/Thermal tabs, full history graph and per-core tiles;
- **Memory** — RAM, SWAP and disk usage;
- **Network** — RX/TX histories, complete kernel counter tables and per-interface visual usage;
- **Processes** — filter, multi-selection, hierarchy indentation and process inspector;
- **Sensors** — available thermal sources and kernel paths;
- **Settings** — polling interval, graph FPS, Y-scale and collection state.

The design uses neutral dark surfaces, a restrained violet accent and monospaced telemetry values. QML layouts are used instead of copying absolute coordinates from the HTML design handoff.

## ⚙️ Sampling and graphs

MoneSys separates three concepts:

- **metric polling** — 250 ms / 500 ms / 1 s / 2 s / 5 s;
- **history window** — 60 seconds by wall clock;
- **graph rendering** — 10 / 30 / 60 FPS.

Changing graph FPS does not increase system polling. Changing polling does not silently shorten the history window.

The CPU/Fan/Thermal group provides the controls required by the assignment:

- Pause/Resume;
- FPS selection;
- `Auto`, `Fixed` and `Peak` Y-scale modes.

Pause stops metric collection without clearing existing history.

## 🌐 Cross-platform design

```text
src/platform/
├── linux/
├── windows/
└── macos/
```

Linux is the first complete provider. Windows and macOS currently compile as explicit telemetry scaffolds rather than fake implementations.

Intended native sources:

- **Windows** — PDH/ETW, IP Helper, Tool Help/NT APIs, DXGI and processor topology APIs;
- **macOS** — Mach host/task statistics, `sysctl`, IOKit and native power/sensor interfaces.

The provider contract, models, sampling controller, history semantics and QML screens do not need to change when those backends are implemented.

## 🧪 Tests and verification

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The initial unit test verifies the time-based history contract.

Agent-navigation metadata has a separate repository contract validator:

```bash
python3 scripts/validate_agent_contracts.py
```

It checks index ↔ manifest integrity, implemented/planned path reality, dependency edges and navigation-size warnings.

## 📋 01-edu audit coverage

The Linux implementation is aligned with the functional checklist from the official assignment:

```text
who
hostname
top
cat /proc/cpuinfo
free -h
df -h /
cat /proc/net/dev
```

The UI contains the required CPU/Fan/Thermal tabs, live graphs, Pause, FPS and Y-scale controls; RAM/SWAP/disk visualization; process columns/filter/multi-selection; IPv4; complete RX/TX tables; live network movement and adaptive byte conversion.

Detailed source-to-audit mapping: [`docs/audit.md`](docs/audit.md).

Official subject: https://github.com/01-edu/public/tree/master/subjects/system-monitor

## 📁 Project structure

```text
system-monitor/
├── agent/
│   ├── modules/
│   ├── schemas/
│   ├── dependency-graph.json
│   ├── methodology.json
│   └── module-index.json
├── docs/
│   ├── architecture.md
│   └── audit.md
├── scripts/
│   └── validate_agent_contracts.py
├── src/
│   ├── core/
│   ├── platform/
│   │   ├── linux/
│   │   ├── windows/
│   │   └── macos/
│   ├── qt/
│   └── main.cpp
├── tests/
│   └── core_tests.cpp
├── ui/
│   ├── components/
│   ├── screens/
│   ├── Main.qml
│   ├── Theme.qml
│   └── Utils.js
├── AGENTS.md
├── CMakeLists.txt
├── README.md
└── README_RU.md
```

## ⚠️ Notes

- The current complete telemetry backend is Linux; Windows/macOS providers are architectural scaffolds, not finished monitoring backends.
- CPU and process values are delta-based. The first sample may legitimately be zero until a second kernel snapshot exists.
- Process files can disappear while `/proc` is being scanned; short-lived processes are skipped rather than treated as fatal errors.
- Sensor availability depends on kernel drivers, permissions, virtualization and hardware exposure.
- Historical audit commands such as `ifconfig` or `/proc/acpi/ibm/thermal` may be absent on modern Linux systems; MoneSys reads generic kernel interfaces directly.
- The design handoff is a visual specification. The HTML reference and landing page are not part of the application runtime.

## 🧑‍💻 Author

- Nazar Yestayev (@nyestaye)
