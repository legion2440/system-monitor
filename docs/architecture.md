# MoneSys architecture

MoneSys is a cross-platform system telemetry product with a platform-neutral C++20 core and a Qt Quick/QML presentation layer.

## Boundaries

```text
QML / Qt Quick
      ↓
Qt bridge + QAbstractItemModel
      ↓
normalized C++20 metric types
      ↓
MetricProvider
  ├── LinuxMetricProvider
  ├── WindowsMetricProvider
  └── MacMetricProvider
```

The core does not include Qt headers. Platform providers do not emit QML objects. The Qt bridge owns the conversion from normalized C++ snapshots into Qt properties and models.

## Metric semantics

- System CPU is normalized to `0..100%` for the whole machine.
- Process CPU uses `100% = one fully occupied logical CPU`, so a multithreaded process can exceed `100%`.
- RAM usage on Linux uses `MemTotal - MemAvailable`.
- Network tables expose cumulative kernel counters from `/proc/net/dev`; separate rate fields are derived from counter deltas.
- Missing hardware capabilities are represented as unavailable, never as synthetic zero values.

## Sampling, history and rendering

These are independent:

1. **sampling cadence** — 250 ms / 500 ms / 1 s / 2 s / 5 s;
2. **history window** — 60 seconds by wall clock;
3. **graph render cadence** — 10 / 30 / 60 FPS.

The history container is time-based, not a fixed `N samples = N seconds` assumption. Changing poll cadence therefore does not silently change the visible history duration.

## Platform providers

### Linux

The first complete backend reads kernel/system interfaces directly:

- `/proc/stat`, `/proc/cpuinfo`;
- `/proc/meminfo`;
- `/proc/<pid>/{stat,cmdline,cgroup,io}`;
- `/proc/net/dev`;
- `getifaddrs()` for IPv4;
- `statvfs()` for root filesystem usage;
- `/sys/class/thermal` and `/sys/class/hwmon` for sensors/fans.

Shell commands are audit/reference tools only, not runtime data sources.

### Windows

The provider boundary already exists. Intended native sources include PDH/ETW, IP Helper, Tool Help/NT APIs, DXGI and `GetLogicalProcessorInformationEx`.

### macOS

The provider boundary already exists. Intended native sources include Mach host/task APIs, `sysctl`, IOKit and native power/sensor interfaces.

## UI

The product UI is implemented natively in Qt Quick/QML from the MoneSys design handoff. The HTML handoff is visual specification only.

The CPU/Fan/Thermal group includes the assignment-required Pause, FPS and Y-scale controls without preserving the old Dear ImGui frontend.
