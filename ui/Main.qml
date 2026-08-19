import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys
import "components"
import "screens"
import "Utils.js" as Utils

ApplicationWindow {
    id: window
    width: 1240
    height: 760
    minimumWidth: 1040
    minimumHeight: 640
    visible: true
    title: "MoneSys"
    color: Theme.bg
    property int section: 0

    background: Rectangle { color: Theme.bg }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.titleBarH
            color: Theme.chrome

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10

                Text {
                    text: "MoneSys"
                    color: Theme.textBright
                    font.family: Theme.uiFont
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }
                Rectangle {
                    width: 5
                    height: 5
                    radius: 3
                    color: app.collecting ? Theme.accent : Theme.textFaint
                    opacity: app.collecting ? 1.0 : 0.45
                }
                Text {
                    text: app.collecting ? qsTr("LIVE") : qsTr("STOPPED")
                    color: app.collecting ? Theme.accent : Theme.textFaint
                    font.family: Theme.monoFont
                    font.pixelSize: 9
                    font.letterSpacing: 1.1
                }
                Text {
                    visible: app.paused
                    text: qsTr("GRAPHS FROZEN")
                    color: Theme.textFaint
                    font.family: Theme.monoFont
                    font.pixelSize: 9
                    font.letterSpacing: 1.1
                }
                Item { Layout.fillWidth: true }
                Text {
                    visible: app.lastError.length > 0
                    text: app.lastError
                    color: "#d99a9a"
                    elide: Text.ElideRight
                    Layout.maximumWidth: 330
                    font.family: Theme.monoFont
                    font.pixelSize: 9
                }
                Text { text: qsTr("poll"); color: Theme.textFaint; font.pixelSize: 9 }
                ComboBox {
                    Layout.preferredWidth: 92
                    model: [250, 500, 1000, 2000, 5000]
                    currentIndex: model.indexOf(app.pollIntervalMs)
                    displayText: currentValue >= 1000 ? (currentValue / 1000) + " s" : currentValue + " ms"
                    delegate: ItemDelegate {
                        required property var modelData
                        width: ListView.view.width
                        text: modelData >= 1000 ? (modelData / 1000) + " s" : modelData + " ms"
                    }
                    onActivated: app.pollIntervalMs = currentValue
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: Theme.sidebarW
                Layout.fillHeight: true
                color: Theme.sidebar

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4
                    Text { Layout.fillWidth: true; Layout.leftMargin: 8; Layout.topMargin: 4; Layout.bottomMargin: 5; text: app.hostname || qsTr("machine"); color: Theme.textFaint; elide: Text.ElideRight; font.family: Theme.monoFont; font.pixelSize: 9 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Overview"); metric: Number(app.cpuUsage).toFixed(0) + "%"; active: window.section === 0; onClicked: window.section = 0 }
                    SidebarItem { Layout.fillWidth: true; label: "CPU"; metric: Utils.ghz(app.cpuFrequencyMHz); active: window.section === 1; onClicked: window.section = 1 }
                    SidebarItem { Layout.fillWidth: true; label: "GPU"; metric: "—"; active: window.section === 2; onClicked: window.section = 2 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Memory"); metric: app.ramTotal > 0 ? (app.ramUsed * 100 / app.ramTotal).toFixed(0) + "%" : "—"; active: window.section === 3; onClicked: window.section = 3 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Disks"); metric: app.diskTotal > 0 ? Math.ceil(Number(app.diskUsagePercent)) + "%" : "—"; active: window.section === 4; onClicked: window.section = 4 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Network"); metric: Utils.rate(app.aggregateRxRate); active: window.section === 5; onClicked: window.section = 5 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Processes"); metric: app.taskTotal.toString(); active: window.section === 6; onClicked: window.section = 6 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Sensors"); metric: app.thermalAvailable ? Number(app.temperature).toFixed(0) + "°" : "—"; active: window.section === 7; onClicked: window.section = 7 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Energy"); metric: "—"; active: window.section === 8; onClicked: window.section = 8 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Services"); metric: "—"; active: window.section === 9; onClicked: window.section = 9 }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Logs"); metric: "—"; active: window.section === 10; onClicked: window.section = 10 }
                    Item { Layout.fillHeight: true }
                    SidebarItem { Layout.fillWidth: true; label: qsTr("Settings"); metric: ""; active: window.section === 11; onClicked: window.section = 11 }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.bg

                StackLayout {
                    anchors.fill: parent
                    currentIndex: window.section

                    Item { OverviewScreen { anchors.fill: parent } }
                    Item { CpuScreen { anchors.fill: parent } }
                    Item { UnavailableScreen { anchors.fill: parent; title: "GPU"; detail: qsTr("GPU is part of the cross-platform provider contract. Linux DRM/NVML/ROCm, Windows DXGI/PDH and macOS Metal/IOKit backends are planned after the 01-edu Linux milestone.") } }
                    Item { MemoryScreen { anchors.fill: parent } }
                    Item { DiskScreen { anchors.fill: parent } }
                    Item { NetworkScreen { anchors.fill: parent } }
                    Item { ProcessesScreen { anchors.fill: parent } }
                    Item { SensorsScreen { anchors.fill: parent } }
                    Item { UnavailableScreen { anchors.fill: parent; title: qsTr("Energy"); detail: qsTr("Linux RAPL capability detection is already exposed by the provider boundary; energy counters will be added as a dedicated normalized metric.") } }
                    Item { UnavailableScreen { anchors.fill: parent; title: qsTr("Services"); detail: qsTr("The provider boundary reserves services for systemd, Windows SCM and launchd without coupling them to QML.") } }
                    Item { UnavailableScreen { anchors.fill: parent; title: qsTr("Logs"); detail: qsTr("Logs are intentionally outside the first Linux audit milestone. The future providers map journald, Windows Event Log and unified logging into one model.") } }
                    Item { SettingsScreen { anchors.fill: parent } }
                }
            }
        }
    }
}
