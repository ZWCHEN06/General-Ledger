pragma ComponentBehavior: Bound

import QtQuick
import "../components"

Item {
    id: root

    signal transactionSaved()
    signal backRequested()

    property string transactionType: "expense"
    property int selectedCategoryId: -1
    property string selectedCategoryName: ""
    property int selectedSubcategoryId: -1
    property string selectedSubcategoryName: ""
    property string errorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 96 : pageMargin
    readonly property int actionHeight: 52

    function refreshCategories() {
        selectedCategoryId = -1
        selectedCategoryName = ""
        selectedSubcategoryId = -1
        selectedSubcategoryName = ""
        appController.subcategoryListModel.clear()
        categoryListModel.refresh(transactionType)
    }

    function selectCategory(categoryId, categoryName) {
        selectedCategoryId = categoryId
        selectedCategoryName = categoryName
        selectedSubcategoryId = -1
        selectedSubcategoryName = ""
        errorMessage = ""

        const result = appController.refreshSubcategories(categoryId)
        if (!result.success) {
            errorMessage = result.errorMessage
        }
    }

    function selectSubcategory(subcategoryId, subcategoryName) {
        selectedSubcategoryId = subcategoryId
        selectedSubcategoryName = subcategoryName
        errorMessage = ""
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

        const result = appController.addTransaction(
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
            transactionSaved()
            return
        }

        errorMessage = result.errorMessage
    }

    Component.onCompleted: {
        if (dateField.text.length === 0) {
            dateField.text = Qt.formatDate(new Date(), "yyyy-MM-dd")
        }

        refreshCategories()
    }

    Flickable {
        id: formFlickable

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: saveButton.top
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
                text: "记一笔"
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
                        root.refreshCategories()
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
                        root.refreshCategories()
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

                        Component.onCompleted: {
                            if (index === 0 && root.selectedCategoryId <= 0) {
                                root.selectCategory(categoryId, name)
                            }
                        }

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
                        required property string name

                        width: (subcategoryGrid.width - subcategoryGrid.columnSpacing) / 2
                        height: 44
                        label: name
                        selected: root.selectedSubcategoryId === model.id

                        Component.onCompleted: {
                            if (index === 0 && root.selectedSubcategoryId <= 0) {
                                root.selectSubcategory(model.id, name)
                            }
                        }

                        onClicked: function(subcategoryName) {
                            root.selectSubcategory(model.id, subcategoryName)
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

    Rectangle {
        id: cancelButton

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: root.pageMargin
        width: 72
        height: 40
        z: 2
        radius: 8
        color: cancelMouseArea.pressed ? "#f1f3f4" : "#ffffff"
        border.color: "#dadce0"

        Text {
            anchors.centerIn: parent
            text: "取消"
            color: "#3c4043"
            font.pixelSize: 15
            font.bold: true
        }

        MouseArea {
            id: cancelMouseArea

            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.backRequested()
        }
    }

    Rectangle {
        id: saveButton

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        height: root.actionHeight
        radius: 8
        color: saveMouseArea.pressed ? "#185abc" : "#1a73e8"

        Text {
            anchors.centerIn: parent
            text: "保存"
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
