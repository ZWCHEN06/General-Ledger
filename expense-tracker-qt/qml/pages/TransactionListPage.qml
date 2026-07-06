pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    signal editTransactionRequested(int transactionId)
    signal backRequested()
    property bool filterExpanded: false
    property bool filterActive: false
    property string selectedFilterType: "all"
    property int selectedFilterCategoryId: -1
    property string selectedFilterCategoryName: ""
    property int selectedFilterSubcategoryId: -1
    property string selectedFilterSubcategoryName: ""
    property int pendingFilterSubcategoryId: -1
    property string filterErrorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function parseMonthFilter() {
        var monthText = monthFilterInput.text.trim()
        if (monthText.length === 0) {
            return {
                "success": true,
                "year": "",
                "month": ""
            }
        }

        var match = /^(\d{4})-(\d{1,2})$/.exec(monthText)
        if (match === null) {
            return {
                "success": false,
                "errorMessage": "月份格式不正确，请使用 YYYY-MM"
            }
        }

        var year = parseInt(match[1], 10)
        var month = parseInt(match[2], 10)
        if (year < 1 || year > 9999) {
            return {
                "success": false,
                "errorMessage": "年份必须是合理数字"
            }
        }

        if (month < 1 || month > 12) {
            return {
                "success": false,
                "errorMessage": "月份必须在 1 到 12 之间"
            }
        }

        return {
            "success": true,
            "year": String(year),
            "month": String(month)
        }
    }

    function applyCurrentFilter() {
        var monthFilter = parseMonthFilter()
        if (!monthFilter.success) {
            filterErrorMessage = monthFilter.errorMessage
            return
        }

        var result = appController.applyTransactionFilter(
                    monthFilter.year,
                    monthFilter.month,
                    selectedFilterType,
                    selectedFilterCategoryName.length > 0 ? selectedFilterCategoryName : categoryFilterInput.text.trim(),
                    selectedFilterCategoryId > 0 ? selectedFilterCategoryId : "",
                    selectedFilterSubcategoryId > 0 ? selectedFilterSubcategoryId : "",
                    keywordFilterInput.text.trim(),
                    minAmountFilterInput.text.trim(),
                    maxAmountFilterInput.text.trim())
        filterErrorMessage = result.success ? "" : result.errorMessage
        if (result.success) {
            filterActive = result.filterActive
        }
    }

    function clearCurrentFilter() {
        monthFilterInput.text = ""
        selectedFilterType = "all"
        categoryFilterInput.text = ""
        selectedFilterCategoryId = -1
        selectedFilterCategoryName = ""
        selectedFilterSubcategoryId = -1
        selectedFilterSubcategoryName = ""
        pendingFilterSubcategoryId = -1
        categoryListModel.refresh("all")
        appController.subcategoryListModel.clear()
        keywordFilterInput.text = ""
        minAmountFilterInput.text = ""
        maxAmountFilterInput.text = ""

        var result = appController.clearTransactionFilter()
        filterErrorMessage = result.success ? "" : result.errorMessage
        if (result.success) {
            filterActive = false
        }
    }

    function restoreCurrentFilter() {
        var state = appController.currentTransactionFilter()
        if (!state.success) {
            filterErrorMessage = state.errorMessage
            return
        }

        if (state.year.length > 0 && state.month.length > 0) {
            var monthNumber = parseInt(state.month, 10)
            monthFilterInput.text = state.year + "-" + (monthNumber < 10 ? "0" + monthNumber : String(monthNumber))
        } else {
            monthFilterInput.text = ""
        }

        selectedFilterType = state.type
        categoryFilterInput.text = state.category
        selectedFilterCategoryId = state.categoryId.length > 0 ? parseInt(state.categoryId, 10) : -1
        selectedFilterCategoryName = state.category
        selectedFilterSubcategoryId = -1
        selectedFilterSubcategoryName = ""
        pendingFilterSubcategoryId = state.subcategoryId.length > 0 ? parseInt(state.subcategoryId, 10) : -1
        keywordFilterInput.text = state.keyword
        minAmountFilterInput.text = state.minAmount
        maxAmountFilterInput.text = state.maxAmount
        filterActive = state.filterActive

        if (selectedFilterType === "income" || selectedFilterType === "expense") {
            categoryListModel.refresh(selectedFilterType)
        } else {
            categoryListModel.refresh("all")
            appController.subcategoryListModel.clear()
        }
    }

    function refreshCurrentList() {
        var result = appController.refreshTransactionList()
        filterErrorMessage = result.success ? "" : result.errorMessage
        if (result.success) {
            filterActive = result.filterActive
        }
    }

    function changeFilterType(type) {
        selectedFilterType = type
        categoryFilterInput.text = ""
        selectedFilterCategoryId = -1
        selectedFilterCategoryName = ""
        selectedFilterSubcategoryId = -1
        selectedFilterSubcategoryName = ""
        pendingFilterSubcategoryId = -1
        filterErrorMessage = ""
        appController.subcategoryListModel.clear()
        categoryListModel.refresh(type)
    }

    function selectFilterCategory(categoryId, categoryName) {
        selectedFilterCategoryId = categoryId
        selectedFilterCategoryName = categoryName
        categoryFilterInput.text = categoryName
        selectedFilterSubcategoryId = -1
        selectedFilterSubcategoryName = ""
        filterErrorMessage = ""

        const result = appController.refreshSubcategories(categoryId)
        if (!result.success) {
            filterErrorMessage = result.errorMessage
        }
    }

    function selectFilterSubcategory(subcategoryId, subcategoryName) {
        selectedFilterSubcategoryId = subcategoryId
        selectedFilterSubcategoryName = subcategoryName
        pendingFilterSubcategoryId = -1
        filterErrorMessage = ""
    }

    Component.onCompleted: {
        if (appController.databaseReady) {
            refreshCurrentList()
        }
    }

    Connections {
        target: appController

        function onDatabaseStatusChanged() {
            if (appController.databaseReady) {
                root.refreshCurrentList()
            }
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

        header: Item {
            id: listHeader

            width: transactionListView.width
            height: titleRow.height + (root.filterExpanded ? filterPanel.height + 12 : 0)

            Component.onCompleted: root.restoreCurrentFilter()

            Item {
                id: titleRow

                width: parent.width
                height: 72

                Text {
                    id: pageTitleText

                    anchors.left: parent.left
                    anchors.right: filterToggleButton.left
                    anchors.rightMargin: 12
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    text: "账单列表"
                    color: "#202124"
                    font.pixelSize: 28
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: filterToggleButton.left
                    anchors.rightMargin: 12
                    anchors.top: pageTitleText.bottom
                    anchors.topMargin: 4
                    text: root.filterActive ? "筛选中，结果 " + transactionListView.count + " 条" : "未筛选"
                    color: root.filterActive ? "#1a73e8" : "#5f6368"
                    font.pixelSize: 14
                    elide: Text.ElideRight
                }

                Rectangle {
                    id: filterToggleButton

                    anchors.right: parent.right
                    anchors.rightMargin: 84
                    anchors.verticalCenter: parent.verticalCenter
                    width: 72
                    height: 40
                    radius: 8
                    color: filterToggleMouseArea.pressed ? "#e8f0fe" : "#ffffff"
                    border.color: root.filterExpanded ? "#1a73e8" : "#dadce0"

                    Text {
                        anchors.centerIn: parent
                        text: root.filterExpanded ? "收起" : "筛选"
                        color: root.filterExpanded ? "#1a73e8" : "#3c4043"
                        font.pixelSize: 15
                        font.bold: true
                    }

                    MouseArea {
                        id: filterToggleMouseArea

                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.filterExpanded = !root.filterExpanded
                    }
                }
            }

            Rectangle {
                id: filterPanel

                anchors.top: titleRow.bottom
                width: parent.width
                height: filterContentColumn.implicitHeight + 24
                visible: root.filterExpanded
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"

                Column {
                    id: filterContentColumn

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    spacing: 10

                    Item {
                        width: parent.width
                        height: 64

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            text: "月份（YYYY-MM）"
                            color: "#3c4043"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 38
                            radius: 6
                            color: "#ffffff"
                            border.color: "#dadce0"

                            TextInput {
                                id: monthFilterInput

                                anchors.fill: parent
                                anchors.margins: 10
                                color: "#202124"
                                font.pixelSize: 16
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 64

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            text: "类型"
                            color: "#3c4043"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Row {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 38
                            spacing: 8

                            Rectangle {
                                width: (parent.width - 16) / 3
                                height: parent.height
                                radius: 6
                                color: root.selectedFilterType === "all" ? "#e8f0fe" : "#ffffff"
                                border.color: root.selectedFilterType === "all" ? "#1a73e8" : "#dadce0"

                                Text {
                                    anchors.centerIn: parent
                                    text: "全部"
                                    color: root.selectedFilterType === "all" ? "#1a73e8" : "#3c4043"
                                    font.pixelSize: 15
                                    font.bold: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.changeFilterType("all")
                                }
                            }

                            Rectangle {
                                width: (parent.width - 16) / 3
                                height: parent.height
                                radius: 6
                                color: root.selectedFilterType === "income" ? "#e8f0fe" : "#ffffff"
                                border.color: root.selectedFilterType === "income" ? "#1a73e8" : "#dadce0"

                                Text {
                                    anchors.centerIn: parent
                                    text: "收入"
                                    color: root.selectedFilterType === "income" ? "#1a73e8" : "#3c4043"
                                    font.pixelSize: 15
                                    font.bold: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.changeFilterType("income")
                                }
                            }

                            Rectangle {
                                width: (parent.width - 16) / 3
                                height: parent.height
                                radius: 6
                                color: root.selectedFilterType === "expense" ? "#e8f0fe" : "#ffffff"
                                border.color: root.selectedFilterType === "expense" ? "#1a73e8" : "#dadce0"

                                Text {
                                    anchors.centerIn: parent
                                    text: "支出"
                                    color: root.selectedFilterType === "expense" ? "#1a73e8" : "#3c4043"
                                    font.pixelSize: 15
                                    font.bold: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.changeFilterType("expense")
                                }
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 64

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            text: "分类"
                            color: "#3c4043"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 38
                            radius: 6
                            color: "#ffffff"
                            border.color: "#dadce0"

                            TextInput {
                                id: categoryFilterInput

                                anchors.fill: parent
                                anchors.margins: 10
                                color: "#202124"
                                font.pixelSize: 16
                                readOnly: true
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: categorySelectionColumn.implicitHeight

                        Column {
                            id: categorySelectionColumn

                            width: parent.width
                            spacing: 8

                            Text {
                                width: parent.width
                                text: "一级分类"
                                color: "#3c4043"
                                font.pixelSize: 14
                                font.bold: true
                            }

                            Text {
                                width: parent.width
                                text: "请先选择收入或支出类型"
                                color: "#5f6368"
                                font.pixelSize: 14
                                wrapMode: Text.WordWrap
                                visible: root.selectedFilterType === "all"
                            }

                            Grid {
                                id: filterCategoryGrid

                                width: parent.width
                                columns: 2
                                columnSpacing: 8
                                rowSpacing: 8
                                visible: root.selectedFilterType === "income" || root.selectedFilterType === "expense"

                                Repeater {
                                    model: categoryListModel

                                    delegate: Rectangle {
                                        required property int categoryId
                                        required property string name

                                        width: (filterCategoryGrid.width - filterCategoryGrid.columnSpacing) / 2
                                        height: 38
                                        radius: 6
                                        color: root.selectedFilterCategoryId === categoryId ? "#e8f0fe" : "#ffffff"
                                        border.color: root.selectedFilterCategoryId === categoryId ? "#1a73e8" : "#dadce0"

                                        Component.onCompleted: {
                                            if ((root.selectedFilterCategoryId > 0 && root.selectedFilterCategoryId === categoryId)
                                                    || (root.selectedFilterCategoryName.length > 0 && root.selectedFilterCategoryName === name)) {
                                                root.selectFilterCategory(categoryId, name)
                                            }
                                        }

                                        Text {
                                            anchors.centerIn: parent
                                            width: parent.width - 16
                                            text: name
                                            color: root.selectedFilterCategoryId === categoryId ? "#1a73e8" : "#3c4043"
                                            font.pixelSize: 14
                                            font.bold: true
                                            horizontalAlignment: Text.AlignHCenter
                                            elide: Text.ElideRight
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: root.selectFilterCategory(categoryId, name)
                                        }
                                    }
                                }
                            }

                            Text {
                                width: parent.width
                                text: "二级分类"
                                color: "#3c4043"
                                font.pixelSize: 14
                                font.bold: true
                                visible: root.selectedFilterCategoryId > 0
                            }

                            Grid {
                                id: filterSubcategoryGrid

                                width: parent.width
                                columns: 2
                                columnSpacing: 8
                                rowSpacing: 8
                                visible: root.selectedFilterCategoryId > 0

                                Rectangle {
                                    width: (filterSubcategoryGrid.width - filterSubcategoryGrid.columnSpacing) / 2
                                    height: 38
                                    radius: 6
                                    color: root.selectedFilterSubcategoryId <= 0 ? "#e8f0fe" : "#ffffff"
                                    border.color: root.selectedFilterSubcategoryId <= 0 ? "#1a73e8" : "#dadce0"

                                    Text {
                                        anchors.centerIn: parent
                                        width: parent.width - 16
                                        text: "全部二级分类"
                                        color: root.selectedFilterSubcategoryId <= 0 ? "#1a73e8" : "#3c4043"
                                        font.pixelSize: 14
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        elide: Text.ElideRight
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: root.selectFilterSubcategory(-1, "")
                                    }
                                }

                                Repeater {
                                    model: appController.subcategoryListModel

                                    delegate: Rectangle {
                                        required property string name

                                        width: (filterSubcategoryGrid.width - filterSubcategoryGrid.columnSpacing) / 2
                                        height: 38
                                        radius: 6
                                        color: root.selectedFilterSubcategoryId === model.id ? "#e8f0fe" : "#ffffff"
                                        border.color: root.selectedFilterSubcategoryId === model.id ? "#1a73e8" : "#dadce0"

                                        Component.onCompleted: {
                                            if (root.pendingFilterSubcategoryId > 0 && root.pendingFilterSubcategoryId === model.id) {
                                                root.selectFilterSubcategory(model.id, name)
                                                return
                                            }

                                            if (root.selectedFilterSubcategoryId === model.id) {
                                                root.selectedFilterSubcategoryName = name
                                            }
                                        }

                                        Text {
                                            anchors.centerIn: parent
                                            width: parent.width - 16
                                            text: name
                                            color: root.selectedFilterSubcategoryId === model.id ? "#1a73e8" : "#3c4043"
                                            font.pixelSize: 14
                                            font.bold: true
                                            horizontalAlignment: Text.AlignHCenter
                                            elide: Text.ElideRight
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: root.selectFilterSubcategory(model.id, name)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 64

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            text: "关键词"
                            color: "#3c4043"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 38
                            radius: 6
                            color: "#ffffff"
                            border.color: "#dadce0"

                            TextInput {
                                id: keywordFilterInput

                                anchors.fill: parent
                                anchors.margins: 10
                                color: "#202124"
                                font.pixelSize: 16
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 64

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            text: "最小金额"
                            color: "#3c4043"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 38
                            radius: 6
                            color: "#ffffff"
                            border.color: "#dadce0"

                            TextInput {
                                id: minAmountFilterInput

                                anchors.fill: parent
                                anchors.margins: 10
                                color: "#202124"
                                font.pixelSize: 16
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 64

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            text: "最大金额"
                            color: "#3c4043"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 38
                            radius: 6
                            color: "#ffffff"
                            border.color: "#dadce0"

                            TextInput {
                                id: maxAmountFilterInput

                                anchors.fill: parent
                                anchors.margins: 10
                                color: "#202124"
                                font.pixelSize: 16
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        height: 44
                        spacing: 10

                        Rectangle {
                            width: (parent.width - 10) / 2
                            height: parent.height
                            radius: 8
                            color: applyFilterMouseArea.pressed ? "#1558b0" : "#1a73e8"

                            Text {
                                anchors.centerIn: parent
                                text: "应用筛选"
                                color: "#ffffff"
                                font.pixelSize: 15
                                font.bold: true
                            }

                            MouseArea {
                                id: applyFilterMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.applyCurrentFilter()
                            }
                        }

                        Rectangle {
                            width: (parent.width - 10) / 2
                            height: parent.height
                            radius: 8
                            color: clearFilterMouseArea.pressed ? "#f1f3f4" : "#ffffff"
                            border.color: "#dadce0"

                            Text {
                                anchors.centerIn: parent
                                text: "清空筛选"
                                color: "#3c4043"
                                font.pixelSize: 15
                                font.bold: true
                            }

                            MouseArea {
                                id: clearFilterMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.clearCurrentFilter()
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        height: root.filterErrorMessage.length > 0 ? implicitHeight : 0
                        text: root.filterErrorMessage
                        color: "#b3261e"
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                        visible: root.filterErrorMessage.length > 0
                    }
                }
            }
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
        text: root.filterActive ? "没有符合条件的账单" : "暂无账单记录"
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
