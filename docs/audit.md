# 01-edu system-monitor audit mapping

MoneSys keeps the original assignment's Linux telemetry semantics while replacing the provided Dear ImGui presentation with a modern Qt Quick/QML frontend.

| Official check | MoneSys source / behavior |
| --- | --- |
| OS name | `/etc/os-release` |
| logged-in user | `getlogin()` / current UID fallback |
| hostname | `gethostname()` |
| total/task states | live scan of `/proc/<pid>/stat` |
| CPU model | `/proc/cpuinfo` model name |
| CPU graph + current % | `/proc/stat` deltas + CPU tab |
| Pause / FPS / Y scale | shared graph controls in CPU/Fan/Thermal view |
| Thermal | `/sys/class/thermal` and hwmon temperature inputs |
| Fan status/speed/level | hwmon `fan*_input`, optional `pwm*` level |
| RAM / SWAP | `/proc/meminfo` |
| Disk | `statvfs("/")` |
| Process PID/name/state/CPU/RAM | `/proc/<pid>` data and deltas |
| Process filter | process screen search field |
| Select multiple processes | selection keyed by PID, persistent across refresh/filter |
| IPv4 | `getifaddrs()` |
| RX/TX raw tables | `/proc/net/dev` counters |
| RX/TX live movement | derived bytes/second history |
| network byte conversion | adaptive B/KB/MB/GB formatting |
| visual range | interface usage view clamps progress range to 0..2 GiB |

The optional build alias `monitor` is a copy of the real `monesys` executable so the historical audit instruction that searches for a process named `monitor` can be reproduced without maintaining a second frontend.

Some commands in the historical checklist are hardware/distribution-specific (`/proc/acpi/ibm/thermal`, `ifconfig`). MoneSys reads generic Linux kernel interfaces and should be compared against the available equivalent on the evaluator machine.
