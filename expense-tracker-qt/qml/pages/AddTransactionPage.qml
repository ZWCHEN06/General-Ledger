pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    property string transactionType: "expense"
    property string selectedCategory: "餐饮"

    readonly property var expenseCategories: ["餐饮", "交通", "购物", "娱乐"]
    readonly property var incomeCategories: ["工资", "奖金", "兼职", "其他"]

    function currentCategories() {
        return transactionType === "income" ? incomeCategories : expenseCategories
    }

    function saveTransaction() {
        console.log("记账类型:", transactionType)
        console.log("金额:", amountField.text)
        console.log("分类:", selectedCategory)
        console.log("日期:", dateField.text)
        console.log("备注:", noteField.text)
    }

    Flickable {
        id: formFlickable

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: saveButton.top
        anchors.margins: 24
        anchors.bottomMargin: 16
        clip: true
        contentWidth: width
        contentHeight: formColumn.height

        Column {
            id: formColumn

            width: formFlickable.width
            spacing: 18

            Text {
                width: parent.width
                text: "记一笔"
                color: "#202124"
                font.pixelSize: 28
                font.bold: true
            }

            Row {
                width: parent.width
                height: 44
                spacing: 12

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
                columnSpacing: 12
                rowSpacing: 12

                Repeater {
                    model: root.currentCategories()
                    delegate: CategoryOption {
                        required property string modelData

                        width: (categoryGrid.width - categoryGrid.columnSpacing) / 2
                        height: 42
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
        }
    }

    Rectangle {
        id: saveButton

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        height: 48
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

        height: 76

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
            anchors.topMargin: 8
            height: 44
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
