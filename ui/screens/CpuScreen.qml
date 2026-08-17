import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys
import "../components"
import "../Utils.js" as Utils

ColumnLayout {
    anchors.fill:parent; anchors.margins:16; spacing:12
    RowLayout { Layout.fillWidth:true; ColumnLayout{Layout.fillWidth:true;spacing:2;Text{text:qsTr("PROCESSOR");color:Theme.accent;font.pixelSize:10;font.letterSpacing:1.4}Text{text:app.cpuModel;color:Theme.textBright;font.pixelSize:17;elide:Text.ElideRight;Layout.fillWidth:true}} Text{text:Utils.ghz(app.cpuFrequencyMHz);color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:13} }
    GraphControls { Layout.fillWidth:true }
    TabBar { id:tabs; Layout.fillWidth:true; TabButton{text:"CPU"} TabButton{text:qsTr("Fan")} TabButton{text:qsTr("Thermal")} }
    StackLayout {
        Layout.fillWidth:true; Layout.fillHeight:true; currentIndex:tabs.currentIndex
        Item { RowLayout { anchors.fill:parent; spacing:14
            Rectangle { Layout.preferredWidth:190; Layout.fillHeight:true; color:Theme.surface; radius:Theme.radiusCard; border.width:1; border.color:Theme.border; ColumnLayout{anchors.fill:parent;anchors.margins:14;Text{text:Number(app.cpuUsage).toFixed(1);color:Theme.accentStep(200);font.family:Theme.monoFont;font.pixelSize:56}Text{text:"% CPU";color:Theme.textMuted;font.family:Theme.monoFont;font.pixelSize:11}Item{Layout.fillHeight:true}Text{text:qsTr("Tasks")+"  "+app.taskTotal;color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:11}Text{text:qsTr("Running")+"  "+app.taskRunning;color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:11}Text{text:qsTr("Blocked")+"  "+app.taskBlocked;color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:11}} }
            ColumnLayout { Layout.fillWidth:true; Layout.fillHeight:true
                Rectangle { Layout.fillWidth:true; Layout.preferredHeight:230; color:Theme.inset; radius:Theme.radiusCard; border.width:1; border.color:Theme.border; HistoryGraph{anchors.fill:parent;anchors.margins:8;samples:app.cpuHistory;fixedMax:100;scaleMode:app.yScaleMode;fps:app.graphFps;paused:app.paused} }
                GridLayout { Layout.fillWidth:true; Layout.fillHeight:true; columns:8; columnSpacing:7; rowSpacing:7; Repeater{model:app.coreUsage;CoreTile{required property real modelData;required property int index;Layout.fillWidth:true;coreIndex:index;usage:modelData;performanceCore:index<Math.ceil(app.coreUsage.length/2)}} }
            }
        } }
        Item { ColumnLayout{anchors.fill:parent;spacing:12;RowLayout{Layout.fillWidth:true;Text{text:app.fanAvailable?app.fanName:qsTr("Fan unavailable on this machine");color:Theme.textBright;font.pixelSize:17;Layout.fillWidth:true}Text{text:app.fanAvailable?Number(app.fanRpm).toFixed(0)+" rpm":"—";color:Theme.accentStep(200);font.family:Theme.monoFont;font.pixelSize:26}}Text{text:app.fanAvailable?(qsTr("status: ")+(app.fanActive?qsTr("active"):qsTr("idle"))+" · "+qsTr("level: ")+(app.fanLevel>=0?Number(app.fanLevel).toFixed(0)+"%":qsTr("unavailable"))):qsTr("No hwmon fan input was exposed by the kernel");color:Theme.textMuted;font.family:Theme.monoFont;font.pixelSize:11}Rectangle{Layout.fillWidth:true;Layout.fillHeight:true;color:Theme.inset;radius:Theme.radiusCard;border.width:1;border.color:Theme.border;HistoryGraph{anchors.fill:parent;anchors.margins:8;samples:app.fanHistory;fixedMax:3000;scaleMode:app.yScaleMode;fps:app.graphFps;lineColor:Theme.neutralRamp[3];fillColor:Qt.rgba(0.7,0.72,0.79,0.12)}}} }
        Item { ColumnLayout{anchors.fill:parent;spacing:12;RowLayout{Layout.fillWidth:true;Text{text:app.thermalAvailable?app.temperatureSource:qsTr("Thermal sensor unavailable");color:Theme.textBright;font.pixelSize:12;Layout.fillWidth:true;elide:Text.ElideMiddle}Text{text:app.thermalAvailable?Number(app.temperature).toFixed(1)+" °C":"—";color:Theme.accentStep(200);font.family:Theme.monoFont;font.pixelSize:26}}Rectangle{Layout.fillWidth:true;Layout.fillHeight:true;color:Theme.inset;radius:Theme.radiusCard;border.width:1;border.color:Theme.border;HistoryGraph{anchors.fill:parent;anchors.margins:8;samples:app.temperatureHistory;fixedMax:95;scaleMode:app.yScaleMode;fps:app.graphFps}}} }
    }
}
