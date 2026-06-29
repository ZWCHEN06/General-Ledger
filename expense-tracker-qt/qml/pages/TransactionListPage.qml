pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    signal editTransactionRequested(int transactionId)

    Component.onCompleted: transactionListModel.refresh()

    ListView {
        id: transactionListView

        anchors.fill: parent
        anchors.margins: 24
        spacing: 12
        clip: true
        model: transactionListModel

        header: Text {
            width: transactionListView.width
            height: 44
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
            height: transactionNote.length > 0 ? 112 : 88
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
                anchors.leftMargin: 18
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
                anchors.rightMargin: 18
                anchors.verticalCenter: typeText.verticalCenter
                width: Math.min(parent.width * 0.38, 150)
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
                anchors.leftMargin: 18
                anchors.right: parent.right
                anchors.rightMargin: 18
                anchors.top: typeText.bottom
                anchors.topMargin: 10
                text: transactionItem.transactionDate
                color: "#5f6368"
                font.pixelSize: 14
                elide: Text.ElideRight
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.right: parent.right
                anchors.rightMargin: 18
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
            height: 12
        }
    }

    Text {
        anchors.centerIn: parent
        text: "暂无账单记录"
        color: "#5f6368"
        font.pixelSize: 18
        visible: transactionListView.count === 0
    }
}
