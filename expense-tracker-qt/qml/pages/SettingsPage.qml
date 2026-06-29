import QtQuick

Item {
    id: root

    property string message: ""

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 24
        spacing: 20

        Text {
            width: parent.width
            text: "设置"
            color: "#202124"
            font.pixelSize: 28
            font.bold: true
        }

        Rectangle {
            id: exportButton

            width: parent.width
            height: 48
            radius: 8
            color: exportMouseArea.pressed ? "#185abc" : "#1a73e8"

            Text {
                anchors.centerIn: parent
                text: "导出 CSV"
                color: "#ffffff"
                font.pixelSize: 18
                font.bold: true
            }

            MouseArea {
                id: exportMouseArea

                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.message = "功能开发中"
            }
        }

        Text {
            width: parent.width
            text: root.message
            color: "#5f6368"
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            visible: root.message.length > 0
        }
    }
}
