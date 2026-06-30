pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    signal editTransactionRequested(int transactionId)
    signal backRequested()
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    Component.onCompleted: {
        if (appController.databaseReady) {
            transactionListModel.refresh()
        }
    }

    ListView {
        id: transactionListView

        anchors.fill: parent
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        spacing: 10
        clip: true
        model: transactionListModel
        visible: appController.databaseReady

        header: Text {
            width: transactionListView.width
            height: 48
            text: "账单列表"
            color: "#202124"
            font.pixelSize: 28
            font.bold: true
        }

        delegate: Rectangle {
            id: transactionItem

            required property string type
            required property int transactionId
            required property real amount
            required property string category
            required property string date
            required property string note

            readonly property string transactionType: type
            readonly property real transactionAmount: amount
            readonly property string transactionCategory: category
            readonly property string transactionDate: date
            readonly property string transactionNote: note
            readonly property bool isIncome: transactionType === "income"

            width: transactionListView.width
            height: transactionNote.length > 0 ? 108 : 84
            radius: 8
            color: "#ffffff"
            border.color: "#e0e0e0"

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.editTransactionRequested(transactionItem.transactionId)
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 4
                radius: 2
                color: transactionItem.isIncome ? "#1b7f45" : "#b3261e"
            }

            Text {
                id: typeText

                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.top: parent.top
                anchors.topMargin: 14
                text: transactionItem.isIncome ? "收入" : "支出"
                color: transactionItem.isIncome ? "#1b7f45" : "#b3261e"
                font.pixelSize: 14
                font.bold: true
            }

            Text {
                id: categoryText

                anchors.left: typeText.right
                anchors.leftMargin: 10
                anchors.right: amountText.left
                anchors.rightMargin: 12
                anchors.verticalCenter: typeText.verticalCenter
                text: transactionItem.transactionCategory
                color: "#202124"
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                id: amountText

                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: typeText.verticalCenter
                width: Math.min(parent.width * 0.42, 150)
                text: (transactionItem.isIncome ? "+" : "-") + "¥" + transactionItem.transactionAmount.toFixed(2)
                color: transactionItem.isIncome ? "#1b7f45" : "#b3261e"
                font.pixelSize: 18
                font.bold: true
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
            }

            Text {
                id: dateText

                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.top: typeText.bottom
                anchors.topMargin: 10
                text: transactionItem.transactionDate
                color: "#5f6368"
                font.pixelSize: 14
                elide: Text.ElideRight
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.top: dateText.bottom
                anchors.topMargin: 8
                text: transactionItem.transactionNote
                color: "#5f6368"
                font.pixelSize: 14
                elide: Text.ElideRight
                visible: transactionItem.transactionNote.length > 0
            }
        }

        footer: Item {
            width: transactionListView.width
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
        text: "暂无账单记录"
        color: "#5f6368"
        font.pixelSize: 18
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        visible: appController.databaseReady && transactionListView.count === 0
    }

    Text {
        anchors.centerIn: parent
        width: Math.max(0, parent.width - root.pageMargin * 2)
        text: appController.databaseErrorMessage
        color: "#b3261e"
        font.pixelSize: 18
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        visible: !appController.databaseReady
    }
}
