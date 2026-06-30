pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    signal transactionSaved()

    property string transactionType: "expense"
    property string selectedCategory: "餐饮"
    property string errorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 96 : pageMargin
    readonly property int actionHeight: 52

    readonly property var expenseCategories: ["餐饮", "交通", "购物", "娱乐"]
    readonly property var incomeCategories: ["工资", "奖金", "兼职", "其他"]

    function currentCategories() {
        return transactionType === "income" ? incomeCategories : expenseCategories
    }

    function saveTransaction() {
        const result = appController.addTransaction(
            transactionType,
            amountField.text,
            selectedCategory,
            dateField.text,
            noteField.text
        )

        if (result.success) {
            errorMessage = ""
            transactionSaved()
            return
        }

        errorMessage = result.errorMessage
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
                }

                TypeOption {
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    label: "收入"
                    value: "income"
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
                    model: root.currentCategories()
                    delegate: CategoryOption {
                        required property string modelData

                        width: (categoryGrid.width - categoryGrid.columnSpacing) / 2
                        height: 44
                        label: modelData
                    }
                }
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

    component TypeOption: Rectangle {
        id: typeOption

        required property string label
        required property string value

        radius: 8
        color: root.transactionType === typeOption.value ? "#e8f0fe" : "#ffffff"
        border.color: root.transactionType === typeOption.value ? "#1a73e8" : "#dadce0"

        Text {
            anchors.centerIn: parent
            text: typeOption.label
            color: root.transactionType === typeOption.value ? "#174ea6" : "#3c4043"
            font.pixelSize: 16
            font.bold: root.transactionType === typeOption.value
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.transactionType = typeOption.value
                root.selectedCategory = root.currentCategories()[0]
            }
        }
    }

    component CategoryOption: Rectangle {
        id: categoryOption

        required property string label

        radius: 8
        color: root.selectedCategory === categoryOption.label ? "#e6f4ea" : "#ffffff"
        border.color: root.selectedCategory === categoryOption.label ? "#1e8e3e" : "#dadce0"

        Text {
            anchors.centerIn: parent
            text: categoryOption.label
            color: root.selectedCategory === categoryOption.label ? "#137333" : "#3c4043"
            font.pixelSize: 15
            font.bold: root.selectedCategory === categoryOption.label
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.selectedCategory = categoryOption.label
        }
    }

    component FormField: Item {
        id: formField

        required property string label
        required property string placeholder
        property int inputMethodHints: Qt.ImhNone
        property alias text: fieldInput.text

        height: 72

        Text {
            id: fieldLabel

            width: parent.width
            text: formField.label
            color: "#3c4043"
            font.pixelSize: 16
            font.bold: true
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: fieldLabel.bottom
            anchors.topMargin: 6
            height: 46
            radius: 8
            color: "#ffffff"
            border.color: fieldInput.activeFocus ? "#1a73e8" : "#dadce0"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: formField.placeholder
                color: "#9aa0a6"
                font.pixelSize: 15
                visible: fieldInput.text.length === 0
            }

            TextInput {
                id: fieldInput

                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                color: "#202124"
                font.pixelSize: 16
                clip: true
                inputMethodHints: formField.inputMethodHints
            }
        }
    }
}
