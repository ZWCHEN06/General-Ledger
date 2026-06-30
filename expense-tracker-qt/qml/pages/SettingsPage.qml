import QtQuick

Item {
    id: root

    property string message: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    Flickable {
        id: settingsFlickable

        anchors.fill: parent
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        clip: true
        contentWidth: width
        contentHeight: settingsColumn.height + root.pageMargin
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: settingsColumn

            width: settingsFlickable.width
            spacing: 16

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
            height: 52
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
            wrapMode: Text.WordWrap
            visible: root.message.length > 0
        }
    }
    }
}
