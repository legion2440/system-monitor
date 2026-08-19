# MoneSys

Современный кроссплатформенный монитор системы и процессов на C++20. MoneSys использует Qt 6 Quick/QML, платформонезависимое ядро метрик и отдельные провайдеры ОС, поэтому Linux, Windows и macOS могут использовать один UI и одну семантику данных без привязки сбора телеметрии к Qt.

Текущий этап реализует Linux-backend для задания 01-edu `system-monitor` и одновременно оставляет готовые границы для будущих нативных провайдеров Windows и macOS.

· [English version](README.md)

## 📋 Содержание

- [🚀 Быстрый старт](#-быстрый-старт)
- [📝 О проекте](#-о-проекте)
- [🏗️ Архитектура](#️-архитектура)
- [📊 Телеметрия Linux](#-телеметрия-linux)
- [🖥️ Интерфейс](#️-интерфейс)
- [⚙️ Опрос и графики](#️-опрос-и-графики)
- [🌐 Кроссплатформенность](#-кроссплатформенность)
- [🧪 Тесты и проверка](#-тесты-и-проверка)
- [📋 Покрытие аудита 01-edu](#-покрытие-аудита-01-edu)
- [📁 Структура проекта](#-структура-проекта)
- [⚠️ Примечания](#️-примечания)
- [🧑‍💻 Автор](#-автор)

## 🚀 Быстрый старт

### Требования

- Linux для текущего полного backend
- компилятор с C++20
- CMake 3.21 или новее
- Qt 6.4 или новее с Qt Quick и Qt Quick Controls 2
- Ninja рекомендуется, но не обязателен

На Ubuntu/Debian нужные пакеты ставятся скриптом:

```bash
./scripts/install-ubuntu-deps.sh
```

### Сборка

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Запуск

```bash
./build/bin/monesys
```

Для старого шага аудита также создаётся идентичный бинарник `monitor`:

```bash
./build/bin/monitor
```

Отключить alias:

```bash
cmake -S . -B build -DMONESYS_BUILD_AUDIT_ALIAS=OFF
```

Если WSLg выбирает нерабочий Wayland socket:

```bash
QT_QPA_PLATFORM=xcb ./build/bin/monesys
```

## 📝 О проекте

MoneSys не является оболочкой над `top`, `free`, `df`, `ifconfig` и другими CLI-утилитами. Linux-provider читает интерфейсы ядра напрямую и отдаёт приложению нормализованные C++ структуры.

Проект намеренно заменяет старый Dear ImGui из задания на современный Qt Quick/QML UI, сохраняя функциональное поведение, которое проверяет audit-list.

Первый этап включает:

- системную информацию и состояния процессов;
- общую загрузку CPU и телеметрию по логическим процессорам;
- RAM, SWAP и root filesystem;
- process CPU/RAM/I/O/state/user;
- фильтр и устойчивый multi-select процессов;
- IPv4 и полные RX/TX kernel counters;
- live network rates и адаптивные единицы;
- thermal/hwmon fan telemetry при наличии;
- 60 секунд истории;
- независимые polling, FPS и Y-scale controls.

GPU, Energy, Services и Logs имеют UI/provider точки, но пока не заявляются как готовая телеметрия.

## 🏗️ Архитектура

```text
Qt Quick / QML
      ↓
Qt bridge + models
      ↓
нормализованные C++20 метрики
      ↓
MetricProvider
  ├── LinuxMetricProvider
  ├── WindowsMetricProvider
  └── MacMetricProvider
```

`src/core/` не зависит от Qt. Платформенные providers возвращают обычные C++ snapshots, а `src/qt/` преобразует их в Qt properties/models для QML.

## 📊 Телеметрия Linux

### Система и CPU

Используются `/etc/os-release`, `gethostname()`, `/proc/cpuinfo`, `/proc/stat` и `/proc/<pid>/stat`.

Общая загрузка CPU нормализована в `0..100%` для всей машины. Для процесса используется семантика `100% = один логический процессор`, поэтому многопоточный процесс может показывать больше `100%`.

### Память и диск

RAM/SWAP читаются из `/proc/meminfo`. Used RAM считается как `MemTotal - MemAvailable`, что совпадает с современным procps.

Диск читается через `statvfs("/")`. `Size`, `Used` и `Avail` соответствуют `df`: `Used = f_blocks - f_bfree`, `Avail = f_bavail`, процент отображается с округлением вверх как у `df`.

### Процессы

Собираются PID/PPID, имя, command line, state, CPU%, memory%, RSS/VIRT, read/write bytes, threads, user и cgroup.

Selection хранится по PID. При выделении или работе со списком вдали от верха порядок строк фиксируется, поэтому строки не прыгают под курсором на каждом poll.

### Сеть

IPv4 берётся через `getifaddrs()`, полные RX/TX counters — из `/proc/net/dev`.

Сырые значения остаются абсолютными и совпадают с kernel counters. Progress bar `0..2 GiB` показывает трафик с момента запуска MoneSys, чтобы движение было видно даже если интерфейс уже давно превысил 2 GiB. KB/MB/GB выводятся с двумя знаками после запятой.

### Температура и вентилятор

Используются только generic Linux interfaces:

- `/sys/class/thermal/thermal_zone*/temp`;
- `/sys/class/hwmon/hwmon*/temp*_input`;
- hwmon `fan*_input`;
- optional `pwm*`.

Имя hwmon-чипа и label конкретного сенсора хранятся отдельно. Основная температура CPU выбирается по приоритету: `Tdie`, затем `Tctl`, package/core labels, затем известные CPU hwmon chips (`k10temp`, `coretemp`, `zenpower`) и только потом общие CPU-названия. Выбор больше не зависит от случайного порядка обхода каталогов. Нулевые, отрицательные и явно нереалистичные температурные значения игнорируются.

Если железо не отдаёт sensor или fan через ядро, UI показывает `Unavailable`, а не подставляет ноль.

## 🖥️ Интерфейс

Текущие экраны:

- **Overview** — CPU, память, tasks, disk, network, sensors, top processes;
- **CPU** — CPU/Fan/Thermal, большой график и logical-processor tiles;
- **Memory** — RAM, SWAP и disk summary;
- **Disks** — root filesystem в семантике `df`;
- **Network** — RX/TX history, полные kernel counter tables и visual usage;
- **Processes** — filter, multi-select, hierarchy, inspector;
- **Sensors** — доступные температурные источники, chip/label и kernel path;
- **Settings** — polling, graph FPS, Y-scale, freeze графиков и состояние collector.

## ⚙️ Опрос и графики

Разделены три вещи:

- **опрос метрик** — 250 мс / 500 мс / 1 с / 2 с / 5 с;
- **окно истории** — 60 секунд;
- **анимация графика** — 10 / 30 / 60 FPS только пока новый sample заезжает в график.

Между sample'ами animation timer не работает. Заморозка графика не останавливает процессы и network counters; полный collector можно остановить отдельно в Settings.

CPU/Fan/Thermal содержат обязательные controls:

- Animation on/off;
- FPS slider;
- Y-scale slider + `Auto`, `Fixed`, `Peak`.

## 🌐 Кроссплатформенность

```text
src/platform/
├── linux/
├── windows/
└── macos/
```

Linux — первый полный provider. Windows/macOS сейчас являются compile-time scaffolds, а не фейковыми реализациями.

## 🧪 Тесты и проверка

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Core test использует явные проверки и exit code, поэтому не исчезает под `-DNDEBUG`.

Agent metadata проверяется отдельно:

```bash
python3 scripts/validate_agent_contracts.py
```

## 📋 Покрытие аудита 01-edu

```text
who
hostname
top
cat /proc/cpuinfo
free -h
df -h /
cat /proc/net/dev
```

В UI есть CPU/Fan/Thermal tabs, live graphs, freeze анимации, рабочие FPS/Y-scale sliders; RAM/SWAP/disk; process columns/filter/multi-select; IPv4; полные RX/TX tables; live network movement и адаптивная конвертация bytes.

Подробное соответствие: [`docs/audit.md`](docs/audit.md).

Официальное задание: https://github.com/01-edu/public/tree/master/subjects/system-monitor

## 📁 Структура проекта

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

## ⚠️ Примечания

- Полный telemetry backend сейчас Linux; Windows/macOS — архитектурные заготовки.
- Первый CPU/process sample может быть нулевым, потому что значения считаются по дельтам.
- Короткоживущие процессы могут исчезать во время чтения `/proc` и безопасно пропускаются.
- Sensors зависят от kernel drivers, permissions, virtualization и hardware exposure.
- Старый audit может ссылаться на утилиты или vendor-specific пути, отсутствующие в современном Linux; MoneSys намеренно использует generic kernel interfaces.
- HTML design handoff и landing page не входят в runtime приложения.

## 🧑‍💻 Автор

- Nazar Yestayev (@nyestaye)
