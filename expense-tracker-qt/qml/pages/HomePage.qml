import QtQuick

Item {
    id: root

    signal addTransactionRequested()
    signal transactionListRequested()
    signal categorySummaryRequested()
    signal weeklyBudgetRequested()
    signal chartPageRequested()
    signal settingsRequested()

    property real monthlyIncome: 0.00
    property real monthlyExpense: 0.00
    property real monthlyBalance: 0.00
    property string errorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 96 : pageMargin
    readonly property int actionHeight: 52

    function refreshSummary() {
        if (!appController.databaseReady) {
            errorMessage = appController.databaseErrorMessage
            monthlyIncome = 0.00
            monthlyExpense = 0.00
            monthlyBalance = 0.00
            return
        }

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

    Flickable {
        id: contentFlickable

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: addButton.top
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.pageMargin
        clip: true
        contentWidth: width
        contentHeight: contentColumn.height
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: contentColumn

            width: contentFlickable.width
            spacing: 16

        Text {
            width: parent.width
            text: "本月概览"
            color: "#202124"
            font.pixelSize: 28
            font.bold: true
        }

        Column {
            width: parent.width
            spacing: 10

            SummaryRow {
                width: parent.width
                title: "本月收入"
                amount: root.monthlyIncome
                amountColor: "#1b7f45"
                visible: root.errorMessage.length === 0
            }

            SummaryRow {
                width: parent.width
                title: "本月支出"
                amount: root.monthlyExpense
                amountColor: "#b3261e"
                visible: root.errorMessage.length === 0
            }

            SummaryRow {
                width: parent.width
                title: "本月结余"
                amount: root.monthlyBalance
                amountColor: "#1f5fbf"
                visible: root.errorMessage.length === 0
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

        Row {
            width: parent.width
            height: 46
            spacing: 10

            NavigationButton {
                width: (parent.width - parent.spacing) / 2
                height: parent.height
                text: "查看账单"
                onClicked: root.transactionListRequested()
            }

            NavigationButton {
                width: (parent.width - parent.spacing) / 2
                height: parent.height
                text: "设置"
                onClicked: root.settingsRequested()
            }
        }

        NavigationButton {
            width: parent.width
            height: 46
            text: "分类统计"
            onClicked: root.categorySummaryRequested()
        }

        NavigationButton {
            width: parent.width
            height: 46
            text: "本周预算"
            onClicked: root.weeklyBudgetRequested()
        }

        NavigationButton {
            width: parent.width
            height: 46
            text: "图表统计"
            onClicked: root.chartPageRequested()
        }
        }
    }

    Rectangle {
        id: addButton

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        height: root.actionHeight
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

        height: 68
        radius: 8
        color: "#ffffff"
        border.color: "#e0e0e0"

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 16
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
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(parent.width * 0.52, 180)
            text: "¥" + summaryRow.amount.toFixed(2)
            color: summaryRow.amountColor
            font.pixelSize: 22
            font.bold: true
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
        }
    }

    component NavigationButton: Rectangle {
        id: navigationButton

        required property string text
        signal clicked()

        radius: 8
        color: navigationMouseArea.pressed ? "#f1f3f4" : "#ffffff"
        border.color: "#dadce0"

        Text {
            anchors.centerIn: parent
            text: navigationButton.text
            color: "#202124"
            font.pixelSize: 16
            font.bold: true
            elide: Text.ElideRight
        }

        MouseArea {
            id: navigationMouseArea

            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: navigationButton.clicked()
        }
    }
}
