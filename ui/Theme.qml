pragma Singleton
import QtQuick

QtObject {
    readonly property color bg: "#161826"
    readonly property color chrome: "#1b1e2c"
    readonly property color sidebar: "#13151f"
    readonly property color surface: "#232532"
    readonly property color inset: "#12141f"
    readonly property color border: "#3f424d"
    readonly property color separator: Qt.rgba(1,1,1,0.07)
    readonly property color rowRule: Qt.rgba(1,1,1,0.06)
    readonly property color hover: Qt.rgba(1,1,1,0.05)
    readonly property color text: "#e9e9ed"
    readonly property color textBright: "#f3f5fe"
    readonly property color textSecond: "#b2b6ca"
    readonly property color textMuted: "#9397ab"
    readonly property color textFaint: "#75798c"
    readonly property color accent: "#9184d9"
    readonly property var accentRamp: ["#f5f4ff","#e7e5fe","#d2cefd","#b5abfc","#968ae0","#796cbf","#5d5294","#423a6a","#2b2741"]
    readonly property var neutralRamp: ["#f3f5fe","#e4e7f5","#cfd3e5","#b2b6ca","#9397ab","#75798c","#595d6c","#3f424d","#292b31"]
    function accentStep(step){ return accentRamp[Math.max(0,Math.min(accentRamp.length-1,step/100-1))] }
    readonly property string uiFont: "Inter"
    readonly property string monoFont: Qt.platform.os === "windows" ? "Consolas" : Qt.platform.os === "osx" ? "SF Mono" : "monospace"
    readonly property int fsKicker: 10
    readonly property int fsMono: 11
    readonly property int fsBody: 13
    readonly property int fsRow: 13
    readonly property int fsKpi: 28
    readonly property int fsHero: 52
    readonly property int titleBarH: 44
    readonly property int sidebarW: 196
    readonly property int detailW: 322
    readonly property int rowH: 34
    readonly property int gap: 14
    readonly property int pad: 16
    readonly property int radiusSm: 4
    readonly property int radiusRow: 6
    readonly property int radiusCard: 8
    readonly property int graphH: 96
}
