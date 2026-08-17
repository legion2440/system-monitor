# MoneSys

Современный кроссплатформенный монитор системы и процессов на C++20. MoneSys использует нативный интерфейс Qt 6 Quick/QML, платформонезависимое ядро метрик и отдельные провайдеры ОС, поэтому Linux, Windows и macOS могут использовать один UI и одну семантику данных без привязки сбора телеметрии к Qt.

Текущий этап полностью реализует Linux-backend для задания 01-edu `system-monitor` и одновременно оставляет готовые границы для будущих нативных провайдеров Windows и macOS.

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

- Linux для текущего полностью реализованного backend
- компилятор с C++20
- CMake 3.21 или новее
- Qt 6.5 или новее с Qt Quick и Qt Quick Controls 2
- Ninja рекомендуется, но не обязателен

Qt 6 development stack можно поставить пакетами своего дистрибутива или официальным Qt installer. Точные имена пакетов отличаются между дистрибутивами.

### Сборка

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Запуск

```bash
./build/monesys
```

По умолчанию сборка также создаёт идентичный executable `monitor` для старого пункта аудита 01-edu, где предлагается найти процесс самого монитора:

```bash
./build/monitor
```

Отключить alias:

```bash
cmake -S . -B build -DMONESYS_BUILD_AUDIT_ALIAS=OFF
```

## 📝 О проекте

MoneSys не является оболочкой над `top`, `free`, `df`, `ifconfig` или другими CLI-утилитами. Linux-provider читает интерфейсы ядра и системы напрямую и передаёт остальному приложению нормализованные C++ структуры метрик.

Проект намеренно заменяет старый Dear ImGui из задания на нативный продуктовый интерфейс Qt Quick/QML, сохраняя поведение телеметрии, которое проверяет audit-list. Под интерфейсом это остаётся C++ проектом системного мониторинга.

Первый этап включает системную информацию, CPU/per-core, RAM/SWAP/disk, процессы, IPv4 и RX/TX counters, live network rates, thermal/fan telemetry, 60 секунд истории и независимые polling/FPS/Y-scale controls.

GPU, Energy, Services и Logs уже имеют UI/provider точки, но пока не заявляются как готовая телеметрия.

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

`src/core/` не содержит Qt headers. Платформенные providers возвращают обычные C++ snapshots. Только `src/qt/` превращает их в Qt properties/models для QML.

Поэтому добавление Windows/macOS telemetry source не требует переписывать UI, историю графиков, представление процессов или общие контракты метрик.

Подробнее: [`docs/architecture.md`](docs/architecture.md).

## 📊 Телеметрия Linux

### Система и CPU

Используются `/etc/os-release`, `gethostname()`, `/proc/cpuinfo`, `/proc/stat` и `/proc/<pid>/stat`.

Общая загрузка CPU нормализована в `0..100%` для всей машины. Для процесса используется семантика `100% = одно логическое ядро`, поэтому многопоточный процесс может показывать больше `100%`.

### Память и диск

- RAM/SWAP — `/proc/meminfo`;
- корневая файловая система — `statvfs("/")`.

Использованная RAM считается как `MemTotal - MemAvailable`.

### Процессы

Модель собирает PID/PPID, имя, command line, state, CPU%, memory%, RSS/VIRT, read/write bytes, threads, user и cgroup.

Selection хранится по PID, а не по номеру строки, поэтому фильтр и обычный refresh не переключают выделение на другой процесс. Можно выбирать несколько строк.

### Сеть

MoneSys объединяет `getifaddrs()` для IPv4 и `/proc/net/dev` для накопительных RX/TX counters. Скорость считается по дельтам счётчиков во времени.

Raw-таблицы сохраняют поля Receive/Transmit из audit-list. UI автоматически преобразует B/KB/MB/GB. Audit-визуализация использует диапазон `0..2 GiB`.

### Температура и вентилятор

Используются общие Linux интерфейсы:

- `/sys/class/thermal/thermal_zone*/temp`;
- `/sys/class/hwmon/hwmon*/temp*_input`;
- hwmon `fan*_input`;
- optional `pwm*` для уровня вентилятора.

Если железо или драйвер не отдаёт sensor/fan, UI показывает `Unavailable`, а не выдуманный ноль.

## 🖥️ Интерфейс

QML-интерфейс следует hi-fi дизайну MoneSys, а не старому скриншоту из задания.

Текущие экраны:

- **Overview** — CPU, память, tasks, disk, network, sensors и top processes;
- **CPU** — вкладки CPU/Fan/Thermal, большой график и per-core tiles;
- **Memory** — RAM, SWAP и disk usage;
- **Network** — RX/TX history, полные kernel counter tables и визуализация интерфейсов;
- **Processes** — filter, multi-select, hierarchy indentation и process inspector;
- **Sensors** — доступные температурные источники;
- **Settings** — polling interval, graph FPS, Y-scale и collection state.

QML использует layouts вместо буквального переноса абсолютных HTML-координат.

## ⚙️ Опрос и графики

В MoneSys разделены:

- **опрос метрик** — 250 мс / 500 мс / 1 с / 2 с / 5 с;
- **окно истории** — 60 секунд реального времени;
- **отрисовка графиков** — 10 / 30 / 60 FPS.

Изменение FPS не заставляет систему собирать метрики 60 раз в секунду. Изменение polling interval не меняет длину history window.

CPU/Fan/Thermal содержат обязательные по заданию controls:

- Pause/Resume;
- выбор FPS;
- Y-scale `Auto`, `Fixed`, `Peak`.

Pause останавливает сбор новых метрик, но не очищает историю.

## 🌐 Кроссплатформенность

```text
src/platform/
├── linux/
├── windows/
└── macos/
```

Linux — первый полный provider. Windows и macOS сейчас являются явными compile-time scaffolds, а не фейковыми реализациями.

Планируемые источники:

- **Windows** — PDH/ETW, IP Helper, Tool Help/NT APIs, DXGI и processor topology APIs;
- **macOS** — Mach host/task statistics, `sysctl`, IOKit и нативные power/sensor interfaces.

Контракт provider, модели данных, sampling controller, history semantics и QML screens при заполнении этих backend менять не требуется.

## 🧪 Тесты и проверка

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Отдельный validator проверяет repo-local agent metadata:

```bash
python3 scripts/validate_agent_contracts.py
```

## 📋 Покрытие аудита 01-edu

Linux implementation соответствует функциональным проверкам официального задания:

```text
who
hostname
top
cat /proc/cpuinfo
free -h
df -h /
cat /proc/net/dev
```

В UI есть CPU/Fan/Thermal tabs, живые графики, Pause, FPS и Y-scale; RAM/SWAP/disk; process columns/filter/multi-select; IPv4; полные RX/TX tables; live network movement и адаптивная конвертация bytes.

Подробное соответствие source → audit: [`docs/audit.md`](docs/audit.md).

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
- CPU/process values считаются по дельтам, поэтому первый sample может быть нулевым.
- Короткоживущие процессы могут исчезнуть во время чтения `/proc` и безопасно пропускаются.
- Sensors зависят от kernel drivers, permissions, virtualization и hardware exposure.
- Старые audit-команды вроде `ifconfig` или `/proc/acpi/ibm/thermal` могут отсутствовать; MoneSys читает общие kernel interfaces напрямую.
- HTML design handoff и landing page не входят в runtime приложения.

## 🧑‍💻 Автор

- Nazar Yestayev (@nyestaye)
