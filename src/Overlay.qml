import QtQuick 2.15

Rectangle {
  anchors.fill: parent
  color: "transparent"

  Rectangle {
    id: panel
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: 40
    radius: 12
    color: Qt.rgba(0, 0, 0, cfg.panelOpacity)

    width: content.implicitWidth + 32
    height: content.implicitHeight + 24

    Text {
      id: content
      anchors.margins: 12
      anchors.fill: parent
      wrapMode: Text.WordWrap

      // Pull from config
      text: (cfg.text && cfg.text.length > 0)
      ? cfg.text
      : "⌨ Keybindings:\n• Super+Enter — Terminal\n• Ctrl+Alt+H — Toggle Overlay"

      font.pixelSize: (cfg.fontSize > 0 ? cfg.fontSize : 28)
      font.family:    (cfg.fontFamily && cfg.fontFamily.length > 0
      ? cfg.fontFamily
      : Qt.application.font.family)   // <- no self-reference

      color: (cfg.textColor && cfg.textColor.length > 0 ? cfg.textColor : "white")
      font.bold: cfg.bold
    }
  }
}
