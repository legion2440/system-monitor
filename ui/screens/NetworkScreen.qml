import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys
import "../components"
import "../Utils.js" as Utils

ColumnLayout {
    anchors.fill: parent
    anchors.margins: 16
    spacing: 10

    RowLayout {
        Layout.fillWidth: true
        Text { text: qsTr("NETWORK"); color: Theme.accent; font.pixelSize: 10; font.letterSpacing: 1.4 }
        Item { Layout.fillWidth: true }
        Text {
            text: "RX " + Utils.rate(app.aggregateRxRate) + "   TX " + Utils.rate(app.aggregateTxRate)
            color: Theme.textSecond
            font.family: Theme.monoFont
            font.pixelSize: 11
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 110
        spacing: 14

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.inset
            radius: Theme.radiusCard
            border.width: 1
            border.color: Theme.border
            HistoryGraph {
                anchors.fill: parent
                anchors.margins: 8
                samples: app.rxHistory
                fixedMax: 104857600
                scaleMode: app.yScaleMode
                scaleMultiplier: app.graphYScale
                fps: app.graphFps
                paused: app.paused
            }
            Text { anchors.left: parent.left; anchors.top: parent.top; anchors.margins: 10; text: "RX"; color: Theme.accent; font.family: Theme.monoFont; font.pixelSize: 10 }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.inset
            radius: Theme.radiusCard
            border.width: 1
            border.color: Theme.border
            HistoryGraph {
                anchors.fill: parent
                anchors.margins: 8
                samples: app.txHistory
                fixedMax: 104857600
                scaleMode: app.yScaleMode
                scaleMultiplier: app.graphYScale
                fps: app.graphFps
                paused: app.paused
                lineColor: Theme.neutralRamp[3]
                fillColor: Qt.rgba(0.7, 0.72, 0.79, 0.10)
            }
            Text { anchors.left: parent.left; anchors.top: parent.top; anchors.margins: 10; text: "TX"; color: Theme.textSecond; font.family: Theme.monoFont; font.pixelSize: 10 }
        }
    }

    TabBar {
        id: counterTabs
        Layout.fillWidth: true
        TabButton { text: "RX" }
        TabButton { text: "TX" }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: counterTabs.currentIndex

        Rectangle {
            color: Theme.surface
            radius: Theme.radiusCard
            border.width: 1
            border.color: Theme.border
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                RowLayout {
                    Layout.fillWidth: true
                    Repeater {
                        model: [qsTr("Interface"), "IPv4", "bytes", "packets", "errs", "drop", "fifo", "frame", "compressed", "multicast"]
                        Text { required property string modelData; Layout.fillWidth: true; text: modelData; color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                    }
                }
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: app.network
                    clip: true
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 30
                        color: index % 2 ? Qt.rgba(1,1,1,0.018) : "transparent"
                        RowLayout {
                            anchors.fill: parent
                            Repeater {
                                model: [name, ipv4 || "—", rxBytes, rxPackets, rxErrors, rxDropped, rxFifo, rxFrame, rxCompressed, rxMulticast]
                                Text { required property var modelData; Layout.fillWidth: true; text: modelData; color: Theme.textSecond; elide: Text.ElideRight; font.family: Theme.monoFont; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            color: Theme.surface
            radius: Theme.radiusCard
            border.width: 1
            border.color: Theme.border
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                RowLayout {
                    Layout.fillWidth: true
                    Repeater {
                        model: [qsTr("Interface"), "IPv4", "bytes", "packets", "errs", "drop", "fifo", "colls", "carrier", "compressed"]
                        Text { required property string modelData; Layout.fillWidth: true; text: modelData; color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                    }
                }
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: app.network
                    clip: true
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 30
                        color: index % 2 ? Qt.rgba(1,1,1,0.018) : "transparent"
                        RowLayout {
                            anchors.fill: parent
                            Repeater {
                                model: [name, ipv4 || "—", txBytes, txPackets, txErrors, txDropped, txFifo, txCollisions, txCarrier, txCompressed]
                                Text { required property var modelData; Layout.fillWidth: true; text: modelData; color: Theme.textSecond; elide: Text.ElideRight; font.family: Theme.monoFont; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                            }
                        }
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Text { text: qsTr("VISUAL USAGE · 0 → 2 GiB SINCE START · TOTALS ARE ABSOLUTE"); color: Theme.textFaint; font.pixelSize: 9; font.letterSpacing: 1.2 }
        Item { Layout.fillWidth: true }
        TabBar {
            id: usageTabs
            Layout.preferredWidth: 220
            TabButton { text: "RX" }
            TabButton { text: "TX" }
        }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 170
        currentIndex: usageTabs.currentIndex

        Rectangle {
            color: Theme.surface
            radius: Theme.radiusCard
            border.width: 1
            border.color: Theme.border
            ListView {
                anchors.fill: parent
                anchors.margins: 12
                model: app.network
                spacing: 8
                clip: true
                delegate: ColumnLayout {
                    width: ListView.view.width
                    spacing: 4
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: name + (ipv4 ? " · " + ipv4 : ""); color: Theme.textBright; font.pixelSize: 11; Layout.fillWidth: true }
                        Text { text: Utils.bytes(rxBytes) + "  (" + rxBytes + " B)"; color: Theme.textMuted; font.family: Theme.monoFont; font.pixelSize: 10 }
                    }
                    ProgressBar { Layout.fillWidth: true; from: 0; to: 2147483648; value: Math.min(2147483648, rxSession) }
                    Text { text: qsTr("session ") + Utils.bytes(rxSession) + " / 2.00 GB"; color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
                }
            }
        }

        Rectangle {
            color: Theme.surface
            radius: Theme.radiusCard
            border.width: 1
            border.color: Theme.border
            ListView {
                anchors.fill: parent
                anchors.margins: 12
                model: app.network
                spacing: 8
                clip: true
                delegate: ColumnLayout {
                    width: ListView.view.width
                    spacing: 4
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: name + (ipv4 ? " · " + ipv4 : ""); color: Theme.textBright; font.pixelSize: 11; Layout.fillWidth: true }
                        Text { text: Utils.bytes(txBytes) + "  (" + txBytes + " B)"; color: Theme.textMuted; font.family: Theme.monoFont; font.pixelSize: 10 }
                    }
                    ProgressBar { Layout.fillWidth: true; from: 0; to: 2147483648; value: Math.min(2147483648, txSession) }
                    Text { text: qsTr("session ") + Utils.bytes(txSession) + " / 2.00 GB"; color: Theme.textFaint; font.family: Theme.monoFont; font.pixelSize: 9 }
                }
            }
        }
    }
}
