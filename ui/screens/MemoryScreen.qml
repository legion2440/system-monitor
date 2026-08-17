import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MoneSys
import "../components"
import "../Utils.js" as Utils
ColumnLayout {
    anchors.fill:parent; anchors.margins:16; spacing:14
    Text{text:qsTr("MEMORY & STORAGE");color:Theme.accent;font.pixelSize:10;font.letterSpacing:1.4}
    GridLayout { Layout.fillWidth:true; columns:3; columnSpacing:14
        MetricCard{Layout.fillWidth:true;Layout.preferredHeight:170;kicker:"RAM";subtitle:Utils.bytes(app.ramUsed)+" / "+Utils.bytes(app.ramTotal);value:app.ramTotal>0?(app.ramUsed*100/app.ramTotal).toFixed(1):"0.0";unit:"%";HistoryGraph{anchors.fill:parent;samples:app.ramHistory;fixedMax:100;scaleMode:"Fixed";fps:app.graphFps;paused:app.paused}}
        MetricCard{Layout.fillWidth:true;Layout.preferredHeight:170;kicker:"SWAP";subtitle:Utils.bytes(app.swapUsed)+" / "+Utils.bytes(app.swapTotal);value:app.swapTotal>0?(app.swapUsed*100/app.swapTotal).toFixed(1):"0.0";unit:"%";Column{anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;spacing:6;Text{text:app.swapTotal>0?qsTr("virtual memory"):qsTr("no swap configured");color:Theme.textFaint;font.family:Theme.monoFont;font.pixelSize:10}ProgressBar{width:parent.width;from:0;to:1;value:app.swapTotal>0?app.swapUsed/app.swapTotal:0}}}
        MetricCard{Layout.fillWidth:true;Layout.preferredHeight:170;kicker:qsTr("DISK");subtitle:"/ · "+Utils.bytes(app.diskUsed)+" / "+Utils.bytes(app.diskTotal);value:app.diskTotal>0?(app.diskUsed*100/app.diskTotal).toFixed(1):"0.0";unit:"%";ProgressBar{anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;from:0;to:1;value:app.diskTotal>0?app.diskUsed/app.diskTotal:0}}
    }
    Rectangle { Layout.fillWidth:true;Layout.fillHeight:true;color:Theme.surface;radius:Theme.radiusCard;border.width:1;border.color:Theme.border;HistoryGraph{anchors.fill:parent;anchors.margins:16;samples:app.ramHistory;fixedMax:100;scaleMode:app.yScaleMode;scaleMultiplier:app.graphYScale;fps:app.graphFps;paused:app.paused} }
}
