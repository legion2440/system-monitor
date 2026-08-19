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
- Qt 6.4 or newer with Qt Quick and Qt Quick Controls 2
- Ninja is recommended but not required

On Ubuntu/Debian the required package set is scripted:

```bash
./scripts/install-ubuntu-deps.sh
```

On other distributions install the Qt 6 development stack from the package manager or the official Qt installer.

### Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run

```bash
./build/bin/monesys
```

By default the build also creates an identical `monitor` executable for the historical 01-edu audit step that asks the evaluator to find the monitor process itself:

```bash
./build/bin/monitor
```

Disable the alias with:

```bash
cmake -S . -B build -DMONESYS_BUILD_AUDIT_ALIAS=OFF
```

For WSLg systems where Qt selects a broken Wayland socket, X11/XCB can be forced explicitly:

```bash
QT_QPA_PLATFORM=xcb ./build/bin/monesys
```

## 📝 About

MoneSys is not a wrapper around `top`, `free`, `df`, `ifconfig` or other command-line tools. The Linux provider reads kernel and system interfaces directly and exposes normalized C++ metric structures to the rest of the application.

The project deliberately replaces the assignment's old Dear ImGui presentation with a native Qt Quick/QML product UI while preserving the telemetry behavior the audit checks. Under the interface it remains a C++ system-monitoring project.

The first milestone includes:

- system identity and process-state counts;
- whole-machine and per-logical-processor CPU telemetry;
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

RAM used follows current procps semantics: `MemTotal - MemAvailable`.

Disk `Size`, `Used` and `Avail` follow `df`: `Used = f_blocks - f_bfree`, `Avail = f_bavail`, and the displayed percentage is rounded up from `Used / (Used + Avail)`.

### Processes

The process model collects PID/PPID, name, command line, state, CPU percentage, memory percentage, RSS, virtual memory, read/write bytes, thread count, user and cgroup data when readable.

Selection is keyed by PID rather than row position. Multiple rows can be selected, and the row order is frozen while the user has a selection or is actively working away from the top of the list.

### Network

MoneSys combines `getifaddrs()` for IPv4 with `/proc/net/dev` for cumulative RX/TX counters. Rates are calculated from timestamped counter deltas.

The raw tables preserve the audit fields for Receive and Transmit. Absolute counters remain byte-identical to the kernel; the `0..2 GiB` progress bars use traffic accumulated since MoneSys started so they still visibly move on long-running machines. User-facing values keep two decimal places for KB/MB/GB conversions.

### Thermal and fan

MoneSys uses generic Linux sensor interfaces:

- `/sys/class/thermal/thermal_zone*/temp`;
- `/sys/class/hwmon/hwmon*/temp*_input`;
- hwmon `fan*_input`;
- optional `pwm*` for fan level.

Hwmon chip and sensor-label metadata are kept separately. The primary CPU temperature is chosen by an ordered preference (`Tdie`, `Tctl`, package/core labels, then known CPU hwmon chips such as `k10temp`, `coretemp` or `zenpower`) instead of depending on filesystem enumeration order. Invalid zero, negative or implausibly high temperature inputs are ignored.

If hardware does not expose a sensor or fan to the kernel, the UI reports it as unavailable instead of inventing a zero value.

## 🖥️ Interface

The Qt Quick/QML UI follows the MoneSys high-fidelity desktop design rather than the original assignment screenshot.

Current screens include:

- **Overview** — CPU, memory, system tasks, disk, network, sensors and top processes;
- **CPU** — CPU/Fan/Thermal tabs, full history graph and logical-processor tiles;
- **Memory** — RAM, SWAP and disk summary;
- **Disks** — root-filesystem `df`-compatible usage and availability;
- **Network** — RX/TX histories, complete kernel counter tables and per-interface visual usage;
- **Processes** — filter, multi-selection, hierarchy indentation and process inspector;
- **Sensors** — available thermal sources, chip/label metadata and kernel paths;
- **Settings** — polling interval, graph FPS, Y-scale, graph freeze and collection state.

## ⚙️ Sampling and graphs

MoneSys separates three concepts:

- **metric polling** — 250 ms / 500 ms / 1 s / 2 s / 5 s;
- **history window** — 60 seconds by wall clock;
- **graph animation** — 10 / 30 / 60 FPS while a new sample slides into view.

Changing graph FPS does not increase system polling. Between samples the graph animation timer is idle.

The CPU/Fan/Thermal group provides the controls required by the assignment:

- animation on/off;
- FPS slider;
- Y-scale slider plus `Auto`, `Fixed` and `Peak` modes.

Freezing animation does not stop process/network telemetry. Full metric collection can be stopped separately in Settings.

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

The core test uses explicit exit-code checks, so it remains meaningful even when `NDEBUG` is defined.

Agent-navigation metadata has a separate repository contract validator:

```bash
python3 scripts/validate_agent_contracts.py
```

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

The UI contains CPU/Fan/Thermal tabs, live graphs, animation freeze, working FPS and Y-scale sliders; RAM/SWAP/disk visualization; process columns/filter/multi-selection; IPv4; complete RX/TX tables; live network movement and adaptive byte conversion.

Detailed source-to-audit mapping: [`docs/audit.md`](docs/audit.md).

Official subject: https://github.com/01-edu/public/tree/master/subjects/system-monitor

## 📁 Project structure

```text
system-monitor/
├── agent/
├── docs/
├── scripts/
├── src/
│   ├── core/
│   ├── platform/
│   │   ├── linux/
│   │   ├── windows/
│   │   └── macos/
│   ├── qt/
│   └── main.cpp
├── tests/
├── ui/
│   ├── components/
│   └── screens/
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
- The historical audit may reference utilities or vendor-specific paths absent on modern Linux; MoneSys intentionally uses generic kernel interfaces.
- The design handoff is a visual specification. The HTML reference and landing page are not part of the application runtime.

## 🧑‍💻 Author

- Nazar Yestayev (@nyestaye)
