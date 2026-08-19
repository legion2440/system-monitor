import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys

ColumnLayout {
    anchors.fill: parent
    anchors.margins: 20
    spacing: 18

    Text { text: qsTr("SETTINGS"); color: Theme.accent; font.pixelSize: 10; font.letterSpacing: 1.4 }
    Text { text: qsTr("Sampling & graphs"); color: Theme.textBright; font.pixelSize: 20 }

    GridLayout {
        columns: 2
        columnSpacing: 28
        rowSpacing: 16
        Layout.fillWidth: true

        Column {
            Text { text: qsTr("Metric polling"); color: Theme.textSecond; font.pixelSize: 12 }
            Text { text: qsTr("Collector cadence. Independent from graph FPS."); color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
        }
        ComboBox {
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

        Column {
            Text { text: qsTr("Graph FPS"); color: Theme.textSecond; font.pixelSize: 12 }
            Text { text: qsTr("Animation cadence only; metric collection remains independent."); color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
        }
        ComboBox {
            model: [10, 30, 60]
            currentIndex: model.indexOf(app.graphFps)
            onActivated: app.graphFps = currentValue
        }

        Column {
            Text { text: qsTr("Y scale"); color: Theme.textSecond; font.pixelSize: 12 }
            Text { text: qsTr("Auto · physical Fixed range · Peak window zoom."); color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
        }
        ComboBox {
            model: ["Auto", "Fixed", "Peak"]
            currentIndex: model.indexOf(app.yScaleMode)
            onActivated: app.yScaleMode = currentValue
        }

        Column {
            Text { text: qsTr("Graph animation"); color: Theme.textSecond; font.pixelSize: 12 }
            Text { text: qsTr("Freezes the curves only. Telemetry, tables and counters keep updating."); color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
        }
        Switch {
            text: app.paused ? qsTr("Frozen") : qsTr("Animating")
            checked: !app.paused
            onToggled: app.paused = !checked
        }

        Column {
            Text { text: qsTr("Metric collection"); color: Theme.textSecond; font.pixelSize: 12 }
            Text { text: qsTr("Stops the sampler entirely. Existing history remains intact."); color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
        }
        Switch {
            text: app.collecting ? qsTr("Live") : qsTr("Stopped")
            checked: app.collecting
            onToggled: app.collecting = checked
        }
    }

    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.separator }
    Text {
        Layout.fillWidth: true
        text: qsTr("MoneSys keeps platform telemetry outside Qt. Linux, Windows and macOS providers implement the same C++ interface; QML only consumes normalized models.")
        color: Theme.textMuted
        wrapMode: Text.WordWrap
        font.pixelSize: 11
    }
    Item { Layout.fillHeight: true }
}
