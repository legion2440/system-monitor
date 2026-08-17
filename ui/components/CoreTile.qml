import QtQuick
import MoneSys
Rectangle {
    property int coreIndex:0; property real usage:0; property bool performanceCore:true
    implicitWidth:88; implicitHeight:54; radius:Theme.radiusSm; color:Theme.inset; border.width:1; border.color:performanceCore?Qt.rgba(0.57,0.52,0.85,0.45):Theme.border
    Rectangle { anchors.left:parent.left; anchors.bottom:parent.bottom; width:parent.width; height:Math.max(1,parent.height*Math.min(1,usage/100)); color:performanceCore?Qt.rgba(0.57,0.52,0.85,0.22):Qt.rgba(0.46,0.47,0.55,0.18); radius:parent.radius }
    Text { anchors.left:parent.left; anchors.leftMargin:7; anchors.top:parent.top; anchors.topMargin:6; text:"CPU "+coreIndex; color:Theme.textFaint; font.family:Theme.monoFont; font.pixelSize:9 }
    Text { anchors.right:parent.right; anchors.rightMargin:7; anchors.bottom:parent.bottom; anchors.bottomMargin:5; text:Number(usage).toFixed(0)+"%"; color:Theme.textBright; font.family:Theme.monoFont; font.pixelSize:13 }
}
