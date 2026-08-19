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
| Pause / FPS / Y scale | shared CPU/Fan/Thermal controls; graph freeze does not stop telemetry |
| Thermal | generic `/sys/class/thermal` and hwmon temperature inputs; CPU source chosen by ordered chip/label priority |
| Fan status/speed/level | hwmon `fan*_input`, optional `pwm*` level |
| RAM / SWAP | `/proc/meminfo` |
| Disk | `statvfs("/")`; `Used`/`Avail` and displayed percentage follow `df` semantics |
| Process PID/name/state/CPU/RAM | `/proc/<pid>` data and deltas |
| Process filter | process screen search field |
| Select multiple processes | selection keyed by PID; order is stabilized while interacting |
| IPv4 | `getifaddrs()` |
| RX/TX raw tables | absolute `/proc/net/dev` counters |
| RX/TX live movement | bytes/second history plus session-delta progress bars |
| network byte conversion | B/KB/MB/GB with two decimals above bytes |
| visual range | session traffic progress range is `0..2 GiB`; absolute counters remain visible separately |

The optional build alias `build/bin/monitor` is a copy of the real `monesys` executable so the historical audit instruction that searches for a process named `monitor` can be reproduced without maintaining a second frontend.

The historical checklist references environment-specific tools and vendor-specific thermal paths. MoneSys intentionally reads generic Linux thermal/hwmon interfaces, so a legacy vendor path can report a different physical sensor even when both readings are valid.
