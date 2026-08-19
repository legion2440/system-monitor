import QtQuick
import QtQuick.Layouts
import MoneSys
import "../Utils.js" as Utils

ColumnLayout {
    anchors.fill: parent
    anchors.margins: 16
    spacing: 14

    Text {
        text: qsTr("DISKS")
        color: Theme.accent
        font.pixelSize: 10
        font.letterSpacing: 1.4
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 220
        color: Theme.surface
        radius: Theme.radiusCard
        border.width: 1
        border.color: Theme.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 14

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3
                    Text {
                        text: qsTr("ROOT FILESYSTEM")
                        color: Theme.textFaint
                        font.family: Theme.monoFont
                        font.pixelSize: 9
                        font.letterSpacing: 1.1
                    }
                    Text {
                        text: "/"
                        color: Theme.textBright
                        font.pixelSize: 20
                    }
                }

                Text {
                    text: app.diskTotal > 0 ? (app.diskUsed * 100 / app.diskTotal).toFixed(1) : "0.0"
                    color: Theme.accentStep(200)
                    font.family: Theme.monoFont
                    font.pixelSize: 42
                }
                Text {
                    text: "%"
                    color: Theme.textMuted
                    font.family: Theme.monoFont
                    font.pixelSize: 13
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 10
                radius: 5
                color: Theme.inset

                Rectangle {
                    width: parent.width * Math.max(0, Math.min(1, app.diskTotal > 0 ? app.diskUsed / app.diskTotal : 0))
                    height: parent.height
                    radius: parent.radius
                    color: Theme.accent
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 24

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text { text: qsTr("USED"); color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
                    Text { text: Utils.bytes(app.diskUsed); color: Theme.textBright; font.family: Theme.monoFont; font.pixelSize: 16 }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text { text: qsTr("FREE"); color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
                    Text { text: Utils.bytes(Math.max(0, app.diskTotal - app.diskUsed)); color: Theme.textBright; font.family: Theme.monoFont; font.pixelSize: 16 }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text { text: qsTr("TOTAL"); color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
                    Text { text: Utils.bytes(app.diskTotal); color: Theme.textBright; font.family: Theme.monoFont; font.pixelSize: 16 }
                }
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: Theme.surface
        radius: Theme.radiusCard
        border.width: 1
        border.color: Theme.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 8

            Text {
                text: qsTr("STORAGE DETAILS")
                color: Theme.textFaint
                font.family: Theme.monoFont
                font.pixelSize: 9
                font.letterSpacing: 1.1
            }
            Text {
                text: qsTr("The current Linux provider reports the root filesystem directly from statvfs('/'). Per-device I/O and mount enumeration are separate telemetry capabilities and will be added without changing this screen contract.")
                color: Theme.textMuted
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                font.pixelSize: 11
            }
            Item { Layout.fillHeight: true }
        }
    }
}
