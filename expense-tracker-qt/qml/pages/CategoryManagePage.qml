pragma ComponentBehavior: Bound

import QtQuick
import "../components"

Item {
    id: root

    signal backRequested()

    property string selectedType: "expense"
    property string errorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function refreshCategories() {
        categoryListModel.refresh(root.selectedType)
    }

    function submitCategory() {
        const categoryName = categoryNameInput.text.trim()
        if (categoryName.length === 0) {
            root.errorMessage = "分类名称不能为空"
            return
        }

        const result = appController.addCategory(categoryName, root.selectedType)
        if (!result.success) {
            root.errorMessage = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "新增分类失败"
            return
        }

        root.errorMessage = ""
        categoryNameInput.text = ""
        root.refreshCategories()
    }

    Component.onCompleted: refreshCategories()

    ListView {
        id: categoryListView

        anchors.fill: parent
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        spacing: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: categoryListModel

        header: Column {
            width: categoryListView.width
            spacing: 16

            Item {
                width: parent.width
                height: 48

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - backButton.width - 12
                    text: "分类管理"
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

            Row {
                width: parent.width
                height: 44
                spacing: 10

                TypeOption {
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    label: "支出"
                    value: "expense"
                    selected: root.selectedType === value
                    onClicked: function(value) {
                        root.selectedType = value
                        root.refreshCategories()
                    }
                }

                TypeOption {
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    label: "收入"
                    value: "income"
                    selected: root.selectedType === value
                    onClicked: function(value) {
                        root.selectedType = value
                        root.refreshCategories()
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: root.errorMessage.length > 0 ? 136 : 108
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Text {
                        width: parent.width
                        text: "新增分类"
                        color: "#202124"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    Row {
                        width: parent.width
                        height: 44
                        spacing: 10

                        Rectangle {
                            width: parent.width - addButton.width - parent.spacing
                            height: parent.height
                            radius: 8
                            color: "#ffffff"
                            border.color: categoryNameInput.activeFocus ? "#1a73e8" : "#dadce0"

                            TextInput {
                                id: categoryNameInput

                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                                color: "#202124"
                                font.pixelSize: 16
                                singleLine: true
                                onAccepted: root.submitCategory()
                            }

                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 12
                                anchors.verticalCenter: parent.verticalCenter
                                text: "输入分类名称"
                                color: "#9aa0a6"
                                font.pixelSize: 16
                                visible: categoryNameInput.text.length === 0
                            }
                        }

                        Rectangle {
                            id: addButton

                            width: 72
                            height: parent.height
                            radius: 8
                            color: addMouseArea.pressed ? "#185abc" : "#1a73e8"

                            Text {
                                anchors.centerIn: parent
                                text: "新增"
                                color: "#ffffff"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            MouseArea {
                                id: addMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.submitCategory()
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        text: root.errorMessage
                        color: "#b3261e"
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                        visible: root.errorMessage.length > 0
                    }
                }
            }

            Item {
                width: parent.width
                height: 2
            }
        }

        delegate: CategoryItem {
            width: categoryListView.width
            name: String(model.name)
            isDefault: Boolean(model.isDefault)
            categoryId: Number(model.id)
        }

        footer: Item {
            width: categoryListView.width
            height: root.bottomInset
        }
    }

    Text {
        anchors.centerIn: parent
        width: Math.max(0, parent.width - root.pageMargin * 2)
        text: "暂无分类"
        color: "#5f6368"
        font.pixelSize: 18
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        visible: categoryListView.count === 0
    }
}
