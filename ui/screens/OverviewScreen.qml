import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys
import "../components"
import "../Utils.js" as Utils

Flickable {
    contentWidth:width; contentHeight:content.implicitHeight+32; clip:true
    ColumnLayout {
        id:content; width:parent.width-32; x:16; y:16; spacing:Theme.gap
        RowLayout {
            Layout.fillWidth:true
            Text { text:qsTr("OVERVIEW"); color:Theme.accent; font.family:Theme.uiFont; font.pixelSize:Theme.fsKicker; font.letterSpacing:1.4 }
            Text { Layout.fillWidth:true; text:app.hostname+" · "+app.osName; color:Theme.textMuted; elide:Text.ElideRight; font.pixelSize:12 }
            Text { text:app.userName; color:Theme.textFaint; font.family:Theme.monoFont; font.pixelSize:10 }
        }
        GridLayout {
            Layout.fillWidth:true; columns:4; columnSpacing:Theme.gap; rowSpacing:Theme.gap
            MetricCard { Layout.columnSpan:2; Layout.fillWidth:true; Layout.preferredHeight:184; kicker:qsTr("PROCESSOR"); subtitle:app.cpuModel; value:Number(app.cpuUsage).toFixed(1); unit:"%"; HistoryGraph { anchors.fill:parent; samples:app.cpuHistory; fixedMax:100; scaleMode:"Fixed"; fps:app.graphFps } }
            MetricCard { Layout.columnSpan:2; Layout.fillWidth:true; Layout.preferredHeight:184; kicker:qsTr("MEMORY"); subtitle:Utils.bytes(app.ramUsed)+" / "+Utils.bytes(app.ramTotal); value:app.ramTotal>0?(app.ramUsed*100/app.ramTotal).toFixed(1):"0.0"; unit:"%"; HistoryGraph { anchors.fill:parent; samples:app.ramHistory; fixedMax:100; scaleMode:"Fixed"; fps:app.graphFps } }
            MetricCard { Layout.fillWidth:true; Layout.preferredHeight:154; kicker:qsTr("SYSTEM"); subtitle:qsTr("tasks"); value:app.taskTotal.toString(); Column { anchors.fill:parent; spacing:4; Text{text:qsTr("running  ")+app.taskRunning;color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:10} Text{text:qsTr("sleeping ")+app.taskSleeping;color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:10} Text{text:qsTr("blocked  ")+app.taskBlocked;color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:10} Text{text:qsTr("zombie   ")+app.taskZombie;color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:10} } }
            MetricCard { Layout.fillWidth:true; Layout.preferredHeight:154; kicker:qsTr("DISK"); subtitle:"/"; value:app.diskTotal>0?(app.diskUsed*100/app.diskTotal).toFixed(1):"0.0"; unit:"%"; Text{anchors.left:parent.left;anchors.bottom:parent.bottom;text:Utils.bytes(app.diskUsed)+" / "+Utils.bytes(app.diskTotal);color:Theme.textMuted;font.family:Theme.monoFont;font.pixelSize:10} }
            MetricCard { Layout.fillWidth:true; Layout.preferredHeight:154; kicker:qsTr("NETWORK"); subtitle:"RX / TX"; value:Utils.rate(app.aggregateRxRate); Column{anchors.fill:parent;spacing:5;Text{text:"RX  "+Utils.rate(app.aggregateRxRate);color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:10}Text{text:"TX  "+Utils.rate(app.aggregateTxRate);color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:10}} }
            MetricCard { Layout.fillWidth:true; Layout.preferredHeight:154; kicker:qsTr("SENSORS"); subtitle:app.thermalAvailable?app.temperatureSource:qsTr("Unavailable"); value:app.thermalAvailable?Number(app.temperature).toFixed(1):"—"; unit:app.thermalAvailable?"°C":""; Text{anchors.left:parent.left;anchors.bottom:parent.bottom;text:app.fanAvailable?((app.fanActive?qsTr("fan active · "):qsTr("fan idle · "))+Number(app.fanRpm).toFixed(0)+" rpm"):qsTr("fan unavailable");color:Theme.textMuted;font.family:Theme.monoFont;font.pixelSize:10} }
        }
        Rectangle {
            Layout.fillWidth:true; Layout.preferredHeight:246; color:Theme.surface; radius:Theme.radiusCard; border.width:1; border.color:Theme.border
            ColumnLayout {
                anchors.fill:parent; anchors.margins:14; spacing:8
                RowLayout { Layout.fillWidth:true; Text{text:qsTr("TOP PROCESSES");color:Theme.accent;font.pixelSize:10;font.letterSpacing:1.4} Item{Layout.fillWidth:true} Text{text:qsTr("100% process CPU = one logical core");color:Theme.textFaint;font.family:Theme.monoFont;font.pixelSize:9} }
                ListView { Layout.fillWidth:true; Layout.fillHeight:true; model:app.processes; interactive:false; clip:true; delegate:Rectangle{width:ListView.view.width;height:29;color:index%2?Qt.rgba(1,1,1,0.018):"transparent";RowLayout{anchors.fill:parent;anchors.leftMargin:8;anchors.rightMargin:8;Text{Layout.preferredWidth:75;text:pid;color:Theme.textFaint;font.family:Theme.monoFont;font.pixelSize:10}Text{Layout.fillWidth:true;text:name;color:Theme.textBright;elide:Text.ElideRight;font.pixelSize:11}Text{Layout.preferredWidth:85;horizontalAlignment:Text.AlignRight;text:Number(cpu).toFixed(1)+"%";color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:10}Text{Layout.preferredWidth:90;horizontalAlignment:Text.AlignRight;text:Number(memory).toFixed(1)+"%";color:Theme.textSecond;font.family:Theme.monoFont;font.pixelSize:10}}} }
            }
        }
    }
}
