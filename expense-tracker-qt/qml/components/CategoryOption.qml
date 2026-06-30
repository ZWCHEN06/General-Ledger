import QtQuick

Rectangle {
    id: categoryOption

    required property string label
    property bool selected: false

    signal clicked(string label)

    radius: 8
    color: selected ? "#e6f4ea" : "#ffffff"
    border.color: selected ? "#1e8e3e" : "#dadce0"

    Text {
        anchors.centerIn: parent
        text: categoryOption.label
        color: categoryOption.selected ? "#137333" : "#3c4043"
        font.pixelSize: 15
        font.bold: categoryOption.selected
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: categoryOption.clicked(categoryOption.label)
    }
}
