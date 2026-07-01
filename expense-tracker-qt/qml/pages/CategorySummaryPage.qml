pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    signal backRequested()

    property var summaryItems: []
    property string errorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function refreshSummary() {
        if (!appController.databaseReady) {
            errorMessage = appController.databaseErrorMessage
            summaryItems = []
            return
        }

        const result = appController.currentMonthCategorySummary()
        if (!result.success) {
            errorMessage = result.errorMessage
            summaryItems = []
            return
        }

        errorMessage = ""
        summaryItems = result.items || []
    }

    Component.onCompleted: refreshSummary()

    ListView {
        id: summaryListView

        anchors.fill: parent
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        spacing: 10
        clip: true
        model: root.summaryItems
        visible: appController.databaseReady && root.errorMessage.length === 0

        header: Text {
            width: summaryListView.width
            height: 56
            text: "本月支出分类排行"
            color: "#202124"
            font.pixelSize: 28
            font.bold: true
        }

        delegate: Rectangle {
            id: summaryItem

            required property int index
            required property var modelData

            readonly property string summaryCategory: modelData.category
            readonly property real summaryAmount: Number(modelData.amount)

            width: summaryListView.width
            height: 68
            radius: 8
            color: "#ffffff"
            border.color: "#e0e0e0"

            Text {
                id: rankText

                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                width: 36
                text: String(summaryItem.index + 1)
                color: "#5f6368"
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                id: categoryText

                anchors.left: rankText.right
                anchors.leftMargin: 10
                anchors.right: amountText.left
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: summaryItem.summaryCategory
                color: "#202124"
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                id: amountText

                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(parent.width * 0.42, 150)
                text: "¥" + summaryItem.summaryAmount.toFixed(2)
                color: "#b3261e"
                font.pixelSize: 18
                font.bold: true
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
            }
        }

        footer: Item {
            width: summaryListView.width
            height: root.bottomInset
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

    Text {
        anchors.centerIn: parent
        width: Math.max(0, parent.width - root.pageMargin * 2)
        text: "暂无支出数据"
        color: "#5f6368"
        font.pixelSize: 18
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        visible: appController.databaseReady
                 && root.errorMessage.length === 0
                 && summaryListView.count === 0
    }

    Text {
        anchors.centerIn: parent
        width: Math.max(0, parent.width - root.pageMargin * 2)
        text: root.errorMessage
        color: "#b3261e"
        font.pixelSize: 18
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        visible: root.errorMessage.length > 0
    }
}
