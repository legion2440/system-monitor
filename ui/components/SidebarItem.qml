import QtQuick
import MoneSys
Rectangle {
    id:root; property string label:""; property string metric:""; property bool active:false; signal clicked()
    implicitHeight:Theme.rowH; radius:Theme.radiusRow; color:active?Qt.rgba(0.57,0.52,0.85,0.15):mouse.containsMouse?Theme.hover:"transparent"
    Rectangle { visible:root.active; width:2; height:parent.height-8; anchors.left:parent.left; anchors.verticalCenter:parent.verticalCenter; radius:1; color:Theme.accent }
    Text { anchors.left:parent.left; anchors.leftMargin:10; anchors.verticalCenter:parent.verticalCenter; text:root.label; color:root.active?Theme.textBright:Theme.textSecond; font.family:Theme.uiFont; font.pixelSize:12 }
    Text { anchors.right:parent.right; anchors.rightMargin:10; anchors.verticalCenter:parent.verticalCenter; text:root.metric; color:Theme.textFaint; font.family:Theme.monoFont; font.pixelSize:10 }
    MouseArea { id:mouse; anchors.fill:parent; hoverEnabled:true; onClicked:root.clicked() }
}
