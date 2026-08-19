import QtQuick
import MoneSys

Rectangle {
    property int threadIndex: 0
    property real usage: 0

    implicitWidth: 92
    implicitHeight: 46
    radius: Theme.radiusSm
    color: Theme.inset
    border.width: 1
    border.color: Theme.border

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width
        height: Math.max(1, parent.height * Math.min(1, usage / 100))
        color: Qt.rgba(0.57, 0.52, 0.85, 0.18)
        radius: parent.radius
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 7
        anchors.top: parent.top
        anchors.topMargin: 5
        text: "THREAD " + (threadIndex + 1)
        color: Theme.textFaint
        font.family: Theme.monoFont
        font.pixelSize: 8
    }

    Text {
        anchors.right: parent.right
        anchors.rightMargin: 7
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
        text: Number(usage).toFixed(0) + "%"
        color: Theme.textBright
        font.family: Theme.monoFont
        font.pixelSize: 12
    }
}
