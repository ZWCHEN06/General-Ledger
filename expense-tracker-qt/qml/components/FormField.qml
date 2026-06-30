import QtQuick

Item {
    id: formField

    required property string label
    required property string placeholder
    property int inputMethodHints: Qt.ImhNone
    property alias text: fieldInput.text
    property alias value: fieldInput.text

    height: 72

    Text {
        id: fieldLabel

        width: parent.width
        text: formField.label
        color: "#3c4043"
        font.pixelSize: 16
        font.bold: true
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: fieldLabel.bottom
        anchors.topMargin: 6
        height: 46
        radius: 8
        color: "#ffffff"
        border.color: fieldInput.activeFocus ? "#1a73e8" : "#dadce0"

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: formField.placeholder
            color: "#9aa0a6"
            font.pixelSize: 15
            visible: fieldInput.text.length === 0
        }

        TextInput {
            id: fieldInput

            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            color: "#202124"
            font.pixelSize: 16
            clip: true
            inputMethodHints: formField.inputMethodHints
        }
    }
}
