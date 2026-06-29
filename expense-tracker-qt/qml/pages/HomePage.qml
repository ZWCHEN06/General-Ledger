import QtQuick

Item {
    id: root

    signal addTransactionRequested()

    property real monthlyIncome: 0.00
    property real monthlyExpense: 0.00
    property real monthlyBalance: 0.00
    property string errorMessage: ""

    function refreshSummary() {
        const result = appController.currentMonthSummary()
        if (!result.success) {
            errorMessage = result.errorMessage
            monthlyIncome = 0.00
            monthlyExpense = 0.00
            monthlyBalance = 0.00
            return
        }

        errorMessage = ""
        monthlyIncome = result.income
        monthlyExpense = result.expense
        monthlyBalance = result.balance
    }

    Component.onCompleted: refreshSummary()

    Column {
        id: contentColumn

        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Text {
            width: parent.width
            text: "本月概览"
            color: "#202124"
            font.pixelSize: 28
            font.bold: true
        }

        Column {
            width: parent.width
            spacing: 12

            SummaryRow {
                width: parent.width
                title: "本月收入"
                amount: root.monthlyIncome
                amountColor: "#1b7f45"
            }

            SummaryRow {
                width: parent.width
                title: "本月支出"
                amount: root.monthlyExpense
                amountColor: "#b3261e"
            }

            SummaryRow {
                width: parent.width
                title: "本月结余"
                amount: root.monthlyBalance
                amountColor: "#1f5fbf"
            }

            Text {
                width: parent.width
                text: root.errorMessage
                color: "#b3261e"
                font.pixelSize: 15
                wrapMode: Text.WordWrap
                visible: root.errorMessage.length > 0
            }
        }
    }

    Rectangle {
        id: addButton

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        height: 48
        radius: 8
        color: addButtonMouseArea.pressed ? "#185abc" : "#1a73e8"

        Text {
            anchors.centerIn: parent
            text: "记一笔"
            color: "#ffffff"
            font.pixelSize: 18
            font.bold: true
        }

        MouseArea {
            id: addButtonMouseArea

            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.addTransactionRequested()
        }
    }

    component SummaryRow: Rectangle {
        id: summaryRow

        required property string title
        required property real amount
        required property color amountColor

        height: 72
        radius: 8
        color: "#ffffff"
        border.color: "#e0e0e0"

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 18
            anchors.right: amountText.left
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: summaryRow.title
            color: "#3c4043"
            font.pixelSize: 18
            elide: Text.ElideRight
        }

        Text {
            id: amountText

            anchors.right: parent.right
            anchors.rightMargin: 18
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(parent.width * 0.48, 180)
            text: "¥" + summaryRow.amount.toFixed(2)
            color: summaryRow.amountColor
            font.pixelSize: 22
            font.bold: true
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
        }
    }
}
