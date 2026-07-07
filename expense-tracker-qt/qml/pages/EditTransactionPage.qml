pragma ComponentBehavior: Bound

import QtQuick
import "../components"

Item {
    id: root

    signal transactionUpdated()
    signal transactionDeleted()

    required property int transactionId

    property string transactionType: "expense"
    property int selectedCategoryId: -1
    property string selectedCategoryName: ""
    property int selectedSubcategoryId: -1
    property string selectedSubcategoryName: ""
    property int pendingCategoryId: -1
    property string pendingCategoryName: ""
    property int pendingSubcategoryId: -1
    property string pendingSubcategoryName: ""
    property bool categorySelectionResolved: false
    property bool subcategorySelectionResolved: false
    property string errorMessage: ""
    property bool confirmDeleteVisible: false
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 96 : pageMargin
    readonly property int actionHeight: 52

    function refreshCategoriesForCurrentType(resetSelection) {
        if (resetSelection) {
            selectedCategoryId = -1
            selectedCategoryName = ""
            selectedSubcategoryId = -1
            selectedSubcategoryName = ""
            pendingCategoryId = -1
            pendingCategoryName = ""
            pendingSubcategoryId = -1
            pendingSubcategoryName = ""
            appController.subcategoryListModel.clear()
        }

        categorySelectionResolved = false
        subcategorySelectionResolved = false
        categoryListModel.refresh(transactionType)
    }

    function refreshSubcategoriesForCurrentCategory(resetSelection) {
        if (resetSelection) {
            selectedSubcategoryId = -1
            selectedSubcategoryName = ""
            pendingSubcategoryId = -1
            pendingSubcategoryName = ""
        }

        subcategorySelectionResolved = false

        if (selectedCategoryId <= 0) {
            appController.subcategoryListModel.clear()
            return
        }

        const result = appController.refreshSubcategories(selectedCategoryId)
        if (!result.success) {
            errorMessage = result.errorMessage
        }
    }

    function applyCategoryCandidate(categoryId, categoryName, index) {
        if (categorySelectionResolved) {
            return
        }

        if (pendingCategoryId > 0 && categoryId === pendingCategoryId) {
            selectedCategoryId = categoryId
            selectedCategoryName = categoryName
            categorySelectionResolved = true
            refreshSubcategoriesForCurrentCategory(false)
            return
        }

        if (pendingCategoryName.length > 0 && categoryName === pendingCategoryName) {
            selectedCategoryId = categoryId
            selectedCategoryName = categoryName
            refreshSubcategoriesForCurrentCategory(false)
            return
        }

        if (index === 0 && selectedCategoryId <= 0) {
            selectedCategoryId = categoryId
            selectedCategoryName = categoryName
            refreshSubcategoriesForCurrentCategory(false)
        }
    }

    function applySubcategoryCandidate(subcategoryId, subcategoryName, index) {
        if (subcategorySelectionResolved) {
            return
        }

        if (pendingSubcategoryId > 0 && subcategoryId === pendingSubcategoryId) {
            selectedSubcategoryId = subcategoryId
            selectedSubcategoryName = subcategoryName
            subcategorySelectionResolved = true
            return
        }

        if (pendingSubcategoryName.length > 0 && subcategoryName === pendingSubcategoryName) {
            selectedSubcategoryId = subcategoryId
            selectedSubcategoryName = subcategoryName
            subcategorySelectionResolved = true
            return
        }

    }

    function selectCategory(categoryId, categoryName) {
        selectedCategoryId = categoryId
        selectedCategoryName = categoryName
        pendingCategoryId = -1
        pendingCategoryName = ""
        categorySelectionResolved = true
        selectedSubcategoryId = -1
        selectedSubcategoryName = ""
        pendingSubcategoryId = -1
        pendingSubcategoryName = ""
        errorMessage = ""
        refreshSubcategoriesForCurrentCategory(false)
    }

    function selectSubcategory(subcategoryId, subcategoryName) {
        selectedSubcategoryId = subcategoryId
        selectedSubcategoryName = subcategoryName
        pendingSubcategoryId = -1
        pendingSubcategoryName = ""
        subcategorySelectionResolved = true
        errorMessage = ""
    }

    function loadTransaction() {
        const result = appController.getTransactionById(transactionId)
        if (!result.success) {
            errorMessage = result.errorMessage
            return
        }

        errorMessage = ""
        transactionType = result.type
        pendingCategoryId = Number(result.categoryId || -1)
        pendingCategoryName = result.category || ""
        pendingSubcategoryId = Number(result.subcategoryId || -1)
        pendingSubcategoryName = result.subcategory || ""
        selectedCategoryId = -1
        selectedCategoryName = ""
        selectedSubcategoryId = -1
        selectedSubcategoryName = ""
        amountField.text = result.amount.toFixed(2)
        dateField.text = result.date
        noteField.text = result.note
        refreshCategoriesForCurrentType(false)
    }

    function saveTransaction() {
        if (selectedCategoryId <= 0 || selectedCategoryName.length === 0) {
            errorMessage = "请先选择分类"
            return
        }

        if (subcategoryRepeater.count === 0) {
            errorMessage = "当前一级分类下没有二级分类，请先在分类管理中添加"
            return
        }

        if (selectedSubcategoryId <= 0 || selectedSubcategoryName.length === 0) {
            errorMessage = "请先选择二级分类"
            return
        }

        const result = appController.updateTransaction(
            transactionId,
            transactionType,
            amountField.text,
            selectedCategoryName,
            dateField.text,
            noteField.text,
            selectedCategoryId,
            selectedSubcategoryId,
            selectedSubcategoryName
        )

        if (result.success) {
            errorMessage = ""
            transactionUpdated()
            return
        }

        errorMessage = result.errorMessage
    }

    function deleteTransaction() {
        const result = appController.deleteTransaction(transactionId)
        if (result.success) {
            errorMessage = ""
            confirmDeleteVisible = false
            transactionDeleted()
            return
        }

        confirmDeleteVisible = false
        errorMessage = result.errorMessage
    }

    Component.onCompleted: loadTransaction()

    Flickable {
        id: formFlickable

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: actionButtons.top
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.pageMargin
        clip: true
        contentWidth: width
        contentHeight: formColumn.height + root.pageMargin
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: formColumn

            width: formFlickable.width
            spacing: 14

            Text {
                width: parent.width
                text: "编辑账单"
                color: "#202124"
                font.pixelSize: 28
                font.bold: true
            }

            Row {
                width: parent.width
                height: 48
                spacing: 10

                TypeOption {
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    label: "支出"
                    value: "expense"
                    selected: root.transactionType === value
                    onClicked: function(selectedType) {
                        root.transactionType = selectedType
                        root.errorMessage = ""
                        root.refreshCategoriesForCurrentType(true)
                    }
                }

                TypeOption {
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    label: "收入"
                    value: "income"
                    selected: root.transactionType === value
                    onClicked: function(selectedType) {
                        root.transactionType = selectedType
                        root.errorMessage = ""
                        root.refreshCategoriesForCurrentType(true)
                    }
                }
            }

            FormField {
                id: amountField

                width: parent.width
                label: "金额"
                placeholder: "请输入金额"
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Text {
                width: parent.width
                text: "分类"
                color: "#3c4043"
                font.pixelSize: 16
                font.bold: true
            }

            Grid {
                id: categoryGrid

                width: parent.width
                columns: 2
                columnSpacing: 10
                rowSpacing: 10

                Repeater {
                    id: categoryRepeater

                    model: categoryListModel

                    delegate: CategoryOption {
                        required property int index
                        required property int categoryId
                        required property string name

                        width: (categoryGrid.width - categoryGrid.columnSpacing) / 2
                        height: 44
                        label: name
                        selected: root.selectedCategoryId === categoryId

                        Component.onCompleted: root.applyCategoryCandidate(categoryId, name, index)

                        onClicked: function(categoryName) {
                            root.selectCategory(categoryId, categoryName)
                        }
                    }
                }
            }

            Text {
                width: parent.width
                text: "暂无可用分类，请先在设置中添加分类"
                color: "#5f6368"
                font.pixelSize: 15
                wrapMode: Text.WordWrap
                visible: categoryRepeater.count === 0
            }

            Text {
                width: parent.width
                text: "二级分类"
                color: "#3c4043"
                font.pixelSize: 16
                font.bold: true
                visible: root.selectedCategoryId > 0
            }

            Grid {
                id: subcategoryGrid

                width: parent.width
                columns: 2
                columnSpacing: 10
                rowSpacing: 10
                visible: root.selectedCategoryId > 0 && subcategoryRepeater.count > 0

                Repeater {
                    id: subcategoryRepeater

                    model: appController.subcategoryListModel

                    delegate: CategoryOption {
                        required property int index
                        required property int subcategoryId
                        required property string name

                        width: (subcategoryGrid.width - subcategoryGrid.columnSpacing) / 2
                        height: 44
                        label: name
                        selected: root.selectedSubcategoryId === subcategoryId

                        Component.onCompleted: root.applySubcategoryCandidate(subcategoryId, name, index)

                        onClicked: function(subcategoryName) {
                            root.selectSubcategory(subcategoryId, subcategoryName)
                        }
                    }
                }
            }

            Text {
                width: parent.width
                text: "当前一级分类下没有二级分类，请先在分类管理中添加"
                color: "#5f6368"
                font.pixelSize: 15
                wrapMode: Text.WordWrap
                visible: root.selectedCategoryId > 0 && subcategoryRepeater.count === 0
            }

            FormField {
                id: dateField

                width: parent.width
                label: "日期"
                placeholder: "YYYY-MM-DD"
            }

            FormField {
                id: noteField

                width: parent.width
                label: "备注"
                placeholder: "可选"
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

    Row {
        id: actionButtons

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        height: root.actionHeight
        spacing: 10

        Rectangle {
            id: deleteButton

            width: (parent.width - parent.spacing) / 2
            height: parent.height
            radius: 8
            color: deleteMouseArea.pressed ? "#a50e0e" : "#b3261e"

            Text {
                anchors.centerIn: parent
                text: "删除"
                color: "#ffffff"
                font.pixelSize: 18
                font.bold: true
            }

            MouseArea {
                id: deleteMouseArea

                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.confirmDeleteVisible = true
            }
        }

        Rectangle {
            id: saveButton

            width: (parent.width - parent.spacing) / 2
            height: parent.height
            radius: 8
            color: saveMouseArea.pressed ? "#185abc" : "#1a73e8"

            Text {
                anchors.centerIn: parent
                text: "保存修改"
                color: "#ffffff"
                font.pixelSize: 18
                font.bold: true
            }

            MouseArea {
                id: saveMouseArea

                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.saveTransaction()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        visible: root.confirmDeleteVisible

        MouseArea {
            anchors.fill: parent
        }

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width - root.pageMargin * 2, 320)
            height: 172
            radius: 8
            color: "#ffffff"
            border.color: "#dadce0"

            Text {
                id: confirmTitle

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 20
                text: "确认删除账单？"
                color: "#202124"
                font.pixelSize: 20
                font.bold: true
            }

            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: confirmTitle.bottom
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                anchors.topMargin: 10
                text: "删除后无法在当前版本中恢复。"
                color: "#5f6368"
                font.pixelSize: 15
                wrapMode: Text.WordWrap
            }

            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 18
                height: 44
                spacing: 10

                Rectangle {
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    radius: 8
                    color: cancelDeleteMouseArea.pressed ? "#f1f3f4" : "#ffffff"
                    border.color: "#dadce0"

                    Text {
                        anchors.centerIn: parent
                        text: "取消"
                        color: "#3c4043"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    MouseArea {
                        id: cancelDeleteMouseArea

                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.confirmDeleteVisible = false
                    }
                }

                Rectangle {
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    radius: 8
                    color: confirmDeleteMouseArea.pressed ? "#a50e0e" : "#b3261e"

                    Text {
                        anchors.centerIn: parent
                        text: "确认删除"
                        color: "#ffffff"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    MouseArea {
                        id: confirmDeleteMouseArea

                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.deleteTransaction()
                    }
                }
            }
        }
    }
}
