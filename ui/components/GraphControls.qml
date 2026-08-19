import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys

Rectangle {
    implicitHeight: 54
    color: Theme.chrome
    radius: Theme.radiusRow
    border.width: 1
    border.color: Theme.border

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 8

        CheckBox {
            text: app.paused ? qsTr("Animation off") : qsTr("Animation on")
            checked: !app.paused
            onToggled: app.paused = !checked
        }

        Text { text: qsTr("FPS") + " " + app.graphFps; color: Theme.textMuted; font.family: Theme.monoFont; font.pixelSize: 10 }
        Slider {
            id: fpsSlider
            Layout.preferredWidth: 118
            from: 0
            to: 2
            stepSize: 1
            snapMode: Slider.SnapAlways
            value: app.graphFps === 10 ? 0 : app.graphFps === 30 ? 1 : 2
            onMoved: app.graphFps = value < 0.5 ? 10 : value < 1.5 ? 30 : 60
        }

        Text { text: qsTr("Y") + " " + Number(app.graphYScale).toFixed(1) + "×"; color: Theme.textMuted; font.family: Theme.monoFont; font.pixelSize: 10 }
        Slider {
            id: ySlider
            Layout.preferredWidth: 142
            from: 0.5
            to: 2.0
            stepSize: 0.1
            value: app.graphYScale
            onMoved: app.graphYScale = value
        }

        ComboBox {
            Layout.preferredWidth: 92
            model: ["Auto", "Fixed", "Peak"]
            currentIndex: model.indexOf(app.yScaleMode)
            onActivated: app.yScaleMode = currentText
        }

        Item { Layout.fillWidth: true }
        Text {
            text: Math.round(1000/app.graphFps) + " ms/frame · 60 s"
            color: Theme.textFaint
            font.family: Theme.monoFont
            font.pixelSize: 9
        }
    }
}
