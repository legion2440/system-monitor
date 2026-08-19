cmake_minimum_required(VERSION 3.21)

project(MoneSys VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MONESYS_BUILD_TESTS "Build MoneSys tests" ON)
option(MONESYS_BUILD_AUDIT_ALIAS "Build a second executable named monitor for the 01-edu audit" ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.4)

add_library(monesys_core STATIC
    src/core/SamplingController.cpp
    src/platform/ProviderFactory.cpp
)
target_include_directories(monesys_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(monesys_core PUBLIC cxx_std_20)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(monesys_core PRIVATE src/platform/linux/LinuxMetricProvider.cpp)
elseif(WIN32)
    target_sources(monesys_core PRIVATE src/platform/windows/WindowsMetricProvider.cpp)
elseif(APPLE)
    target_sources(monesys_core PRIVATE src/platform/macos/MacMetricProvider.cpp)
else()
    message(FATAL_ERROR "MoneSys currently supports Linux, Windows and macOS targets")
endif()

add_library(monesys_qt STATIC
    src/qt/AppController.cpp
    src/qt/ProcessModel.cpp
    src/qt/NetworkModel.cpp
    src/qt/SensorModel.cpp
)
target_include_directories(monesys_qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(monesys_qt PUBLIC monesys_core Qt6::Quick Qt6::QuickControls2)

# Keep source files grouped under ui/ while exposing the intended QML module layout.
set_source_files_properties(ui/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
set_source_files_properties(ui/Theme.qml PROPERTIES QT_RESOURCE_ALIAS Theme.qml QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(ui/Utils.js PROPERTIES QT_RESOURCE_ALIAS Utils.js)
set_source_files_properties(ui/components/MetricCard.qml PROPERTIES QT_RESOURCE_ALIAS components/MetricCard.qml)
set_source_files_properties(ui/components/HistoryGraph.qml PROPERTIES QT_RESOURCE_ALIAS components/HistoryGraph.qml)
set_source_files_properties(ui/components/GraphControls.qml PROPERTIES QT_RESOURCE_ALIAS components/GraphControls.qml)
set_source_files_properties(ui/components/CoreTile.qml PROPERTIES QT_RESOURCE_ALIAS components/CoreTile.qml)
set_source_files_properties(ui/components/SidebarItem.qml PROPERTIES QT_RESOURCE_ALIAS components/SidebarItem.qml)
set_source_files_properties(ui/screens/OverviewScreen.qml PROPERTIES QT_RESOURCE_ALIAS screens/OverviewScreen.qml)
set_source_files_properties(ui/screens/CpuScreen.qml PROPERTIES QT_RESOURCE_ALIAS screens/CpuScreen.qml)
set_source_files_properties(ui/screens/MemoryScreen.qml PROPERTIES QT_RESOURCE_ALIAS screens/MemoryScreen.qml)
set_source_files_properties(ui/screens/NetworkScreen.qml PROPERTIES QT_RESOURCE_ALIAS screens/NetworkScreen.qml)
set_source_files_properties(ui/screens/ProcessesScreen.qml PROPERTIES QT_RESOURCE_ALIAS screens/ProcessesScreen.qml)
set_source_files_properties(ui/screens/SensorsScreen.qml PROPERTIES QT_RESOURCE_ALIAS screens/SensorsScreen.qml)
set_source_files_properties(ui/screens/SettingsScreen.qml PROPERTIES QT_RESOURCE_ALIAS screens/SettingsScreen.qml)
set_source_files_properties(ui/screens/UnavailableScreen.qml PROPERTIES QT_RESOURCE_ALIAS screens/UnavailableScreen.qml)

# Qt 6.8+ generates these redirects automatically via QTP0004. Keep explicit
# redirects for the minimum supported Qt 6.4 so files in subdirectories share
# the MoneSys implicit import and can resolve the Theme singleton reliably.
set_source_files_properties(ui/components/qmldir PROPERTIES QT_RESOURCE_ALIAS components/qmldir)
set_source_files_properties(ui/screens/qmldir PROPERTIES QT_RESOURCE_ALIAS screens/qmldir)

qt_add_executable(monesys src/main.cpp)
qt_add_qml_module(monesys
    URI MoneSys
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        ui/Main.qml
        ui/Theme.qml
        ui/Utils.js
        ui/components/MetricCard.qml
        ui/components/HistoryGraph.qml
        ui/components/GraphControls.qml
        ui/components/CoreTile.qml
        ui/components/SidebarItem.qml
        ui/screens/OverviewScreen.qml
        ui/screens/CpuScreen.qml
        ui/screens/MemoryScreen.qml
        ui/screens/NetworkScreen.qml
        ui/screens/ProcessesScreen.qml
        ui/screens/SensorsScreen.qml
        ui/screens/SettingsScreen.qml
        ui/screens/UnavailableScreen.qml
    RESOURCES
        ui/components/qmldir
        ui/screens/qmldir
)
target_link_libraries(monesys PRIVATE monesys_qt Qt6::Quick Qt6::QuickControls2)
set_target_properties(monesys PROPERTIES
    OUTPUT_NAME monesys
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(MONESYS_BUILD_AUDIT_ALIAS)
    add_custom_command(TARGET monesys POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:monesys>
            $<TARGET_FILE_DIR:monesys>/monitor${CMAKE_EXECUTABLE_SUFFIX}
        COMMENT "Creating monitor audit alias")
endif()

if(MONESYS_BUILD_TESTS)
    enable_testing()
    add_executable(monesys_core_tests tests/core_tests.cpp)
    target_link_libraries(monesys_core_tests PRIVATE monesys_core)
    add_test(NAME monesys_core_tests COMMAND monesys_core_tests)
endif()
