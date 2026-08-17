import QtQuick
import QtQuick.Layouts
import MoneSys

Rectangle {
    id: card
    property string kicker: ""
    property string subtitle: ""
    property string value: ""
    property string unit: ""
    default property alias content: body.data
    color: Theme.surface; radius: Theme.radiusCard; border.width: 1; border.color: Theme.border
    implicitHeight: Math.max(156,layout.implicitHeight+Theme.pad*2)
    ColumnLayout {
        id: layout; anchors.fill: parent; anchors.margins: Theme.pad; spacing: 12
        RowLayout {
            Layout.fillWidth: true; spacing: 10
            Text { text: card.kicker; color: Theme.accent; font.family: Theme.uiFont; font.pixelSize: Theme.fsKicker; font.letterSpacing: 1.4; font.capitalization: Font.AllUppercase }
            Text { Layout.fillWidth: true; text: card.subtitle; color: Theme.textMuted; elide: Text.ElideRight; font.family: Theme.uiFont; font.pixelSize: 12 }
            Text { text: card.value; color: Theme.accentStep(200); font.family: Theme.monoFont; font.pixelSize: Theme.fsKpi; font.letterSpacing: -0.6 }
            Text { text: card.unit; color: Theme.textMuted; font.family: Theme.monoFont; font.pixelSize: 13 }
        }
        Item { id: body; Layout.fillWidth: true; Layout.fillHeight: true; implicitHeight: childrenRect.height }
    }
}
