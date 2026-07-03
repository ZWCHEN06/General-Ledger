import QtQuick

Item {
    id: root

    signal backRequested()

    property date selectedWeekStartDate: getWeekStartDate(new Date())
    property string errorMessage: ""
    property bool budgetDialogVisible: false
    property bool deleteConfirmVisible: false
    property int editingCategoryId: -1
    property string editingCategoryName: ""
    property string budgetAmountText: ""
    property string budgetDialogError: ""
    property int deletingCategoryId: -1
    property string deletingCategoryName: ""
    property string deleteErrorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function pad2(value) {
        return value < 10 ? "0" + value : String(value)
    }

    function formatDate(date) {
        return date.getFullYear() + "-" + pad2(date.getMonth() + 1) + "-" + pad2(date.getDate())
    }

    function formatMoney(value) {
        return Number(value).toFixed(2)
    }

    function usagePercentText() {
        if (Number(appController.totalBudget) <= 0) {
            return "未设置"
        }

        return Number(appController.totalUsagePercent).toFixed(2) + "%"
    }

    function itemUsagePercentText(hasBudget, usagePercent) {
        if (!hasBudget) {
            return "未设置预算"
        }

        return Number(usagePercent).toFixed(2) + "%"
    }

    function addDays(date, days) {
        const result = new Date(date)
        result.setDate(result.getDate() + days)
        return result
    }

    function getWeekStartDate(date) {
        const result = new Date(date)
        const day = result.getDay()
        const offset = day === 0 ? -6 : 1 - day
        result.setHours(0, 0, 0, 0)
        result.setDate(result.getDate() + offset)
        return result
    }

    function weekRangeText() {
        const weekEndDate = addDays(root.selectedWeekStartDate, 6)
        return formatDate(root.selectedWeekStartDate) + " ~ " + formatDate(weekEndDate)
    }

    function currentWeekStartText() {
        return formatDate(root.selectedWeekStartDate)
    }

    function loadWeeklyBudget() {
        const result = appController.loadWeeklyBudget(root.currentWeekStartText())
        if (!result.success) {
            root.errorMessage = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "加载每周预算失败"
            return
        }

        root.errorMessage = ""
    }

    function goPreviousWeek() {
        root.selectedWeekStartDate = addDays(root.selectedWeekStartDate, -7)
        root.closeBudgetDialog()
        root.closeDeleteConfirm()
        root.loadWeeklyBudget()
    }

    function goNextWeek() {
        root.selectedWeekStartDate = addDays(root.selectedWeekStartDate, 7)
        root.closeBudgetDialog()
        root.closeDeleteConfirm()
        root.loadWeeklyBudget()
    }

    function openBudgetDialog(categoryId, categoryName, hasBudget, budgetAmount) {
        root.closeDeleteConfirm()
        root.editingCategoryId = categoryId
        root.editingCategoryName = categoryName
        root.budgetAmountText = hasBudget ? root.formatMoney(budgetAmount) : ""
        root.budgetDialogError = ""
        root.budgetDialogVisible = true
        budgetInput.forceActiveFocus()
    }

    function closeBudgetDialog() {
        root.budgetDialogVisible = false
        root.editingCategoryId = -1
        root.editingCategoryName = ""
        root.budgetAmountText = ""
        root.budgetDialogError = ""
    }

    function saveBudget() {
        const trimmedAmount = root.budgetAmountText.trim()
        if (trimmedAmount.length === 0) {
            root.budgetDialogError = "预算金额不能为空"
            return
        }

        const amount = Number(trimmedAmount)
        if (isNaN(amount)) {
            root.budgetDialogError = "预算金额必须是数字"
            return
        }

        if (amount < 0) {
            root.budgetDialogError = "预算金额不能小于 0"
            return
        }

        const result = appController.setWeeklyBudget(
                    root.currentWeekStartText(),
                    root.editingCategoryId,
                    amount)
        if (!result.success) {
            root.budgetDialogError = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "保存预算失败"
            return
        }

        root.closeBudgetDialog()
        root.loadWeeklyBudget()
    }

    function openDeleteConfirm(categoryId, categoryName) {
        root.closeBudgetDialog()
        root.deletingCategoryId = categoryId
        root.deletingCategoryName = categoryName
        root.deleteErrorMessage = ""
        root.deleteConfirmVisible = true
    }

    function closeDeleteConfirm() {
        root.deleteConfirmVisible = false
        root.deletingCategoryId = -1
        root.deletingCategoryName = ""
        root.deleteErrorMessage = ""
    }

    function confirmDeleteBudget() {
        if (root.deletingCategoryId <= 0) {
            root.closeDeleteConfirm()
            return
        }

        const result = appController.deleteWeeklyBudget(
                    root.currentWeekStartText(),
                    root.deletingCategoryId)
        if (!result.success) {
            root.deleteErrorMessage = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "删除预算失败"
            return
        }

        root.closeDeleteConfirm()
        root.loadWeeklyBudget()
    }

    Component.onCompleted: root.loadWeeklyBudget()

    Flickable {
        id: budgetFlickable

        anchors.fill: parent
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        clip: true
        contentWidth: width
        contentHeight: budgetColumn.height + root.pageMargin
        boundsBehavior: Flickable.StopAtBounds
        enabled: !root.budgetDialogVisible && !root.deleteConfirmVisible

        Column {
            id: budgetColumn

            width: budgetFlickable.width
            spacing: 16

            Item {
                width: parent.width
                height: 48

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - backButton.width - 12
                    text: "本周预算"
                    color: "#202124"
                    font.pixelSize: 28
                    font.bold: true
                    elide: Text.ElideRight
                }

                Rectangle {
                    id: backButton

                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 72
                    height: 40
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

            Rectangle {
                width: parent.width
                height: 132
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    Text {
                        width: parent.width
                        text: root.weekRangeText()
                        color: "#202124"
                        font.pixelSize: 22
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }

                    Row {
                        width: parent.width
                        height: 44
                        spacing: 10

                        WeekButton {
                            width: (parent.width - parent.spacing) / 2
                            height: parent.height
                            text: "上一周"
                            onClicked: root.goPreviousWeek()
                        }

                        WeekButton {
                            width: (parent.width - parent.spacing) / 2
                            height: parent.height
                            text: "下一周"
                            onClicked: root.goNextWeek()
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: root.errorMessage.length > 0 ? 112 : 300
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"

                Text {
                    anchors.centerIn: parent
                    width: parent.width - 32
                    text: root.errorMessage
                    color: "#b3261e"
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    visible: root.errorMessage.length > 0
                }

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12
                    visible: root.errorMessage.length === 0

                    Text {
                        width: parent.width
                        text: "预算总览"
                        color: "#202124"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    SummaryRow {
                        width: parent.width
                        title: "本周总预算"
                        value: "¥" + root.formatMoney(appController.totalBudget)
                        valueColor: "#1f5fbf"
                    }

                    SummaryRow {
                        width: parent.width
                        title: "本周实际支出"
                        value: "¥" + root.formatMoney(appController.totalActual)
                        valueColor: "#b3261e"
                    }

                    SummaryRow {
                        width: parent.width
                        title: "本周剩余"
                        value: "¥" + root.formatMoney(appController.totalRemaining)
                        valueColor: appController.totalRemaining < 0 ? "#b3261e" : "#1b7f45"
                    }

                    SummaryRow {
                        width: parent.width
                        title: "使用比例"
                        value: root.usagePercentText()
                        valueColor: appController.isTotalOverBudget ? "#b3261e" : "#3c4043"
                    }

                    Text {
                        width: parent.width
                        text: "本周已超支"
                        color: "#b3261e"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        visible: appController.isTotalOverBudget
                    }
                }
            }

            Text {
                width: parent.width
                text: "分类预算"
                color: "#202124"
                font.pixelSize: 18
                font.bold: true
                visible: root.errorMessage.length === 0
            }

            ListView {
                id: categoryBudgetListView

                width: parent.width
                height: contentHeight
                spacing: 10
                interactive: false
                boundsBehavior: Flickable.StopAtBounds
                model: appController.weeklyBudgetListModel
                visible: root.errorMessage.length === 0

                delegate: Rectangle {
                    id: budgetItem

                    required property int categoryId
                    required property string categoryName
                    required property real budgetAmount
                    required property real actualAmount
                    required property real remainingAmount
                    required property real usagePercent
                    required property bool isOverBudget
                    required property bool hasBudget

                    width: categoryBudgetListView.width
                    height: budgetItem.hasBudget ? 206 : 164
                    radius: 8
                    color: itemMouseArea.pressed ? "#f8f9fa" : "#ffffff"
                    border.color: budgetItem.isOverBudget ? "#f4b4ae" : "#dadce0"

                    MouseArea {
                        id: itemMouseArea

                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.openBudgetDialog(
                                       budgetItem.categoryId,
                                       budgetItem.categoryName,
                                       budgetItem.hasBudget,
                                       budgetItem.budgetAmount)
                    }

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Row {
                            width: parent.width
                            height: 24
                            spacing: 10

                            Text {
                                width: budgetItem.isOverBudget ? parent.width - overBudgetText.width - parent.spacing : parent.width
                                text: budgetItem.categoryName
                                color: "#202124"
                                font.pixelSize: 17
                                font.bold: true
                                elide: Text.ElideRight
                            }

                            Text {
                                id: overBudgetText

                                width: 58
                                text: "已超支"
                                color: "#b3261e"
                                font.pixelSize: 14
                                font.bold: true
                                horizontalAlignment: Text.AlignRight
                                visible: budgetItem.isOverBudget
                            }
                        }

                        Column {
                            width: parent.width
                            spacing: 8

                            DetailRow {
                                width: parent.width
                                title: "预算金额"
                                value: budgetItem.hasBudget ? "¥" + root.formatMoney(budgetItem.budgetAmount) : "未设置预算"
                                valueColor: budgetItem.hasBudget ? "#1f5fbf" : "#5f6368"
                            }

                            DetailRow {
                                width: parent.width
                                title: "实际支出"
                                value: "¥" + root.formatMoney(budgetItem.actualAmount)
                                valueColor: "#b3261e"
                            }

                            DetailRow {
                                width: parent.width
                                title: "剩余金额"
                                value: budgetItem.hasBudget ? "¥" + root.formatMoney(budgetItem.remainingAmount) : "未设置预算"
                                valueColor: !budgetItem.hasBudget ? "#5f6368"
                                        : budgetItem.remainingAmount < 0 ? "#b3261e" : "#1b7f45"
                            }

                            DetailRow {
                                width: parent.width
                                title: "使用比例"
                                value: root.itemUsagePercentText(budgetItem.hasBudget, budgetItem.usagePercent)
                                valueColor: budgetItem.isOverBudget ? "#b3261e" : "#3c4043"
                            }
                        }

                        Rectangle {
                            width: parent.width
                            height: 36
                            radius: 8
                            color: deleteBudgetMouseArea.pressed ? "#fce8e6" : "#ffffff"
                            border.color: "#f4b4ae"
                            visible: budgetItem.hasBudget

                            Text {
                                anchors.centerIn: parent
                                text: "删除预算"
                                color: "#b3261e"
                                font.pixelSize: 15
                                font.bold: true
                            }

                            MouseArea {
                                id: deleteBudgetMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.openDeleteConfirm(
                                               budgetItem.categoryId,
                                               budgetItem.categoryName)
                            }
                        }
                    }
                }
            }

            Text {
                width: parent.width
                text: "暂无支出分类"
                color: "#5f6368"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                visible: root.errorMessage.length === 0 && categoryBudgetListView.count === 0
            }

            Item {
                width: parent.width
                height: root.bottomInset
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        z: 10
        color: "#80000000"
        visible: root.budgetDialogVisible

        MouseArea {
            anchors.fill: parent
            onClicked: root.closeBudgetDialog()
        }

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width - root.pageMargin * 2, 340)
            height: root.budgetDialogError.length > 0 ? 236 : 204
            radius: 8
            color: "#ffffff"
            border.color: "#dadce0"

            MouseArea {
                anchors.fill: parent
            }

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12

                Text {
                    width: parent.width
                    text: "设置预算"
                    color: "#202124"
                    font.pixelSize: 20
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: root.editingCategoryName
                    color: "#5f6368"
                    font.pixelSize: 15
                    elide: Text.ElideRight
                }

                Rectangle {
                    width: parent.width
                    height: 44
                    radius: 8
                    color: "#ffffff"
                    border.color: budgetInput.activeFocus ? "#1a73e8" : "#dadce0"

                    TextInput {
                        id: budgetInput

                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        verticalAlignment: TextInput.AlignVCenter
                        clip: true
                        color: "#202124"
                        font.pixelSize: 16
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        text: root.budgetAmountText
                        onTextChanged: {
                            if (root.budgetAmountText !== text) {
                                root.budgetAmountText = text
                            }
                        }
                        onAccepted: root.saveBudget()
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: "输入预算金额"
                        color: "#9aa0a6"
                        font.pixelSize: 16
                        visible: budgetInput.text.length === 0
                    }
                }

                Text {
                    width: parent.width
                    text: root.budgetDialogError
                    color: "#b3261e"
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    visible: root.budgetDialogError.length > 0
                }

                DialogButtonRow {
                    width: parent.width
                    cancelText: "取消"
                    confirmText: "保存"
                    confirmColor: "#1a73e8"
                    confirmPressedColor: "#185abc"
                    onCancelClicked: root.closeBudgetDialog()
                    onConfirmClicked: root.saveBudget()
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        z: 11
        color: "#80000000"
        visible: root.deleteConfirmVisible

        MouseArea {
            anchors.fill: parent
            onClicked: root.closeDeleteConfirm()
        }

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width - root.pageMargin * 2, 340)
            height: root.deleteErrorMessage.length > 0 ? 218 : 184
            radius: 8
            color: "#ffffff"
            border.color: "#dadce0"

            MouseArea {
                anchors.fill: parent
            }

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12

                Text {
                    width: parent.width
                    text: "确认删除预算"
                    color: "#202124"
                    font.pixelSize: 20
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: "确定删除“" + root.deletingCategoryName + "”的本周预算吗？"
                    color: "#5f6368"
                    font.pixelSize: 15
                    wrapMode: Text.WordWrap
                }

                Text {
                    width: parent.width
                    text: root.deleteErrorMessage
                    color: "#b3261e"
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    visible: root.deleteErrorMessage.length > 0
                }

                DialogButtonRow {
                    width: parent.width
                    cancelText: "取消"
                    confirmText: "删除"
                    confirmColor: "#d93025"
                    confirmPressedColor: "#a50e0e"
                    onCancelClicked: root.closeDeleteConfirm()
                    onConfirmClicked: root.confirmDeleteBudget()
                }
            }
        }
    }

    component WeekButton: Rectangle {
        id: weekButton

        required property string text
        signal clicked()

        radius: 8
        color: weekButtonMouseArea.pressed ? "#f1f3f4" : "#ffffff"
        border.color: "#dadce0"

        Text {
            anchors.centerIn: parent
            text: weekButton.text
            color: "#3c4043"
            font.pixelSize: 16
            font.bold: true
        }

        MouseArea {
            id: weekButtonMouseArea

            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: weekButton.clicked()
        }
    }

    component DialogButtonRow: Row {
        id: dialogButtonRow

        required property string cancelText
        required property string confirmText
        required property color confirmColor
        required property color confirmPressedColor
        signal cancelClicked()
        signal confirmClicked()

        height: 42
        spacing: 10

        Rectangle {
            width: (parent.width - parent.spacing) / 2
            height: parent.height
            radius: 8
            color: cancelMouseArea.pressed ? "#f1f3f4" : "#ffffff"
            border.color: "#dadce0"

            Text {
                anchors.centerIn: parent
                text: dialogButtonRow.cancelText
                color: "#3c4043"
                font.pixelSize: 16
                font.bold: true
            }

            MouseArea {
                id: cancelMouseArea

                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: dialogButtonRow.cancelClicked()
            }
        }

        Rectangle {
            width: (parent.width - parent.spacing) / 2
            height: parent.height
            radius: 8
            color: confirmMouseArea.pressed ? dialogButtonRow.confirmPressedColor : dialogButtonRow.confirmColor

            Text {
                anchors.centerIn: parent
                text: dialogButtonRow.confirmText
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
            }

            MouseArea {
                id: confirmMouseArea

                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: dialogButtonRow.confirmClicked()
            }
        }
    }

    component SummaryRow: Rectangle {
        id: summaryRow

        required property string title
        required property string value
        required property color valueColor

        height: 44
        radius: 8
        color: "#f8f9fa"
        border.color: "#e8eaed"

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: valueText.left
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: summaryRow.title
            color: "#3c4043"
            font.pixelSize: 15
            elide: Text.ElideRight
        }

        Text {
            id: valueText

            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(parent.width * 0.48, 160)
            text: summaryRow.value
            color: summaryRow.valueColor
            font.pixelSize: 16
            font.bold: true
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
        }
    }

    component DetailRow: Item {
        id: detailRow

        required property string title
        required property string value
        required property color valueColor

        height: 18

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * 0.45
            text: detailRow.title
            color: "#5f6368"
            font.pixelSize: 14
            elide: Text.ElideRight
        }

        Text {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * 0.5
            text: detailRow.value
            color: detailRow.valueColor
            font.pixelSize: 14
            font.bold: true
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
        }
    }
}
