import QtQuick

Rectangle {
    id: typeOption

    required property string label
    required property string value
    property bool selected: false

    signal clicked(string value)

    radius: 8
    color: selected ? "#e8f0fe" : "#ffffff"
    border.color: selected ? "#1a73e8" : "#dadce0"

    Text {
        anchors.centerIn: parent
        text: typeOption.label
        color: typeOption.selected ? "#174ea6" : "#3c4043"
        font.pixelSize: 16
        font.bold: typeOption.selected
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: typeOption.clicked(typeOption.value)
    }
}
