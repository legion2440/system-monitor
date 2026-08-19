import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys

Rectangle {
    id: controls
    implicitHeight: 54
    color: Theme.chrome
    radius: Theme.radiusRow
    border.width: 1
    border.color: Theme.border
    clip: true

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 8

        CheckBox {
            id: animationToggle
            Layout.preferredWidth: 126
            Layout.minimumWidth: 110
            text: qsTr("Animate graph")
            checked: !app.paused
            onToggled: app.paused = !checked

            contentItem: Text {
                leftPadding: animationToggle.indicator.width + animationToggle.spacing
                text: animationToggle.text
                color: Theme.textBright
                font.pixelSize: 11
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }

        Text {
            text: qsTr("FPS") + " " + app.graphFps
            color: Theme.textMuted
            font.family: Theme.monoFont
            font.pixelSize: 10
        }
        Slider {
            id: fpsSlider
            Layout.preferredWidth: 110
            Layout.minimumWidth: 80
            from: 0
            to: 2
            stepSize: 1
            snapMode: Slider.SnapAlways
            value: app.graphFps === 10 ? 0 : app.graphFps === 30 ? 1 : 2
            onMoved: app.graphFps = value < 0.5 ? 10 : value < 1.5 ? 30 : 60
        }

        Text {
            text: qsTr("Y") + " " + Number(app.graphYScale).toFixed(1) + "×"
            color: Theme.textMuted
            font.family: Theme.monoFont
            font.pixelSize: 10
        }
        Slider {
            id: ySlider
            Layout.preferredWidth: 120
            Layout.minimumWidth: 85
            from: 0.5
            to: 2.0
            stepSize: 0.1
            value: app.graphYScale
            onMoved: app.graphYScale = value
        }

        ComboBox {
            Layout.preferredWidth: 88
            Layout.minimumWidth: 82
            model: ["Auto", "Fixed", "Peak"]
            currentIndex: model.indexOf(app.yScaleMode)
            onActivated: app.yScaleMode = currentText
        }

        Item { Layout.fillWidth: true; Layout.minimumWidth: 0 }
        Text {
            visible: controls.width >= 820
            text: Math.round(1000/app.graphFps) + " ms/frame · 60 s"
            color: Theme.textFaint
            font.family: Theme.monoFont
            font.pixelSize: 9
        }
    }
}
