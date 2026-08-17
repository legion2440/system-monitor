import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys

Rectangle {
    implicitHeight: 42; color: Theme.chrome; radius: Theme.radiusRow; border.width: 1; border.color: Theme.border
    RowLayout {
        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 10
        Button { text: app.paused ? qsTr("Resume") : qsTr("Pause"); onClicked: app.paused=!app.paused }
        Text { text: qsTr("FPS"); color: Theme.textMuted; font.pixelSize: 11 }
        ComboBox { model:[10,30,60]; currentIndex: app.graphFps===10?0:app.graphFps===30?1:2; onActivated: app.graphFps=Number(currentText) }
        Text { text: qsTr("Y scale"); color: Theme.textMuted; font.pixelSize: 11 }
        ComboBox { model:["Auto","Fixed","Peak"]; currentIndex:model.indexOf(app.yScaleMode); onActivated:app.yScaleMode=currentText }
        Item { Layout.fillWidth: true }
        Text { text: app.graphFps+" fps · "+Math.round(1000/app.graphFps)+" ms/frame · 60 s history"; color:Theme.textFaint; font.family:Theme.monoFont; font.pixelSize:10 }
    }
}
