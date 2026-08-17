import QtQuick
import QtQuick.Layouts
import MoneSys
Item {
    property string title:""
    property string detail:qsTr("This module has a UI slot and provider contract, but telemetry is not implemented for the current milestone.")
    ColumnLayout{anchors.centerIn:parent;width:Math.min(parent.width-80,520);spacing:10;Text{Layout.fillWidth:true;text:parent.parent.title;color:Theme.textBright;font.pixelSize:24;horizontalAlignment:Text.AlignHCenter}Text{Layout.fillWidth:true;text:parent.parent.detail;color:Theme.textMuted;font.pixelSize:12;wrapMode:Text.WordWrap;horizontalAlignment:Text.AlignHCenter}}
}
