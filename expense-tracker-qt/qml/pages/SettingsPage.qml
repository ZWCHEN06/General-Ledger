import QtQuick

Item {
    id: root

    signal backRequested()
    signal categoryManageRequested()

    property string message: ""
    property bool messageIsError: false
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function exportCsv() {
        const result = appController.exportCsv()
        if (result.success) {
            message = "CSV 已导出：" + result.filePath
            messageIsError = false
            return
        }

        message = result.errorMessage
        messageIsError = true
    }

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
                    onClicked: root.exportCsv()
                }
            }

            Rectangle {
                id: categoryManageButton

                width: parent.width
                height: 52
                radius: 8
                color: categoryManageMouseArea.pressed ? "#f1f3f4" : "#ffffff"
                border.color: "#dadce0"

                Text {
                    anchors.centerIn: parent
                    text: "分类管理"
                    color: "#3c4043"
                    font.pixelSize: 18
                    font.bold: true
                }

                MouseArea {
                    id: categoryManageMouseArea

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.categoryManageRequested()
                }
            }

            Text {
                width: parent.width
                text: root.message
                color: root.messageIsError ? "#b3261e" : "#5f6368"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                visible: root.message.length > 0
            }
        }
    }

    Rectangle {
        id: backButton

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: root.pageMargin
        width: 72
        height: 40
        z: 2
        radius: 8
        color: backMouseArea.pressed ? "#f1f3f4" : "#ffffff"
        border.color: "#dadce0"

        Text {
            anchors.centerIn: parent
            text: "返回"
            color: "#3c4043"
            font.pixelSize: 15
            font.bold: true
        }

        MouseArea {
            id: backMouseArea

            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.backRequested()
        }
    }
}
