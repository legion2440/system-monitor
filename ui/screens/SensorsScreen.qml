import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys
ColumnLayout {
    anchors.fill:parent;anchors.margins:16;spacing:12
    Text{text:qsTr("SENSORS");color:Theme.accent;font.pixelSize:10;font.letterSpacing:1.4}
    Rectangle{Layout.fillWidth:true;Layout.fillHeight:true;color:Theme.surface;radius:Theme.radiusCard;border.width:1;border.color:Theme.border;ListView{anchors.fill:parent;anchors.margins:10;model:app.sensors;spacing:1;clip:true;delegate:Rectangle{width:ListView.view.width;height:44;color:index%2?Qt.rgba(1,1,1,0.018):"transparent";RowLayout{anchors.fill:parent;anchors.leftMargin:9;anchors.rightMargin:9;Text{Layout.preferredWidth:250;text:chip.length>0?chip+" · "+name:name;color:Theme.textBright;font.pixelSize:12;elide:Text.ElideRight}Text{Layout.fillWidth:true;text:source;color:Theme.textFaint;font.family:Theme.monoFont;font.pixelSize:9;elide:Text.ElideMiddle}Text{Layout.preferredWidth:100;text:Number(value).toFixed(1)+" "+unit;color:Theme.accentStep(200);font.family:Theme.monoFont;font.pixelSize:13;horizontalAlignment:Text.AlignRight}}}}}
}
