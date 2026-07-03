pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    signal backRequested()

    property int categoryId: -1
    property string categoryName: ""
    property string newSubcategoryName: ""
    property string errorMessage: ""
    property int editingSubcategoryId: -1
    property string editingSubcategoryName: ""
    property string editingSubcategoryDraftName: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function refreshSubcategories() {
        if (root.categoryId <= 0) {
            root.errorMessage = "一级分类 id 无效"
            appController.subcategoryListModel.clear()
            return
        }

        const result = appController.refreshSubcategories(root.categoryId)
        if (!result.success) {
            root.errorMessage = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "刷新二级分类失败"
            return
        }

        root.errorMessage = ""
    }

    function submitSubcategory() {
        const subcategoryName = root.newSubcategoryName.trim()
        if (subcategoryName.length === 0) {
            root.errorMessage = "二级分类名称不能为空"
            return
        }

        if (root.categoryId <= 0) {
            root.errorMessage = "一级分类 id 无效"
            return
        }

        const result = appController.addSubcategory(root.categoryId, subcategoryName)
        if (!result.success) {
            root.errorMessage = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "新增二级分类失败"
            return
        }

        root.errorMessage = ""
        root.newSubcategoryName = ""
        root.refreshSubcategories()
    }

    function cancelEdit() {
        root.editingSubcategoryId = -1
        root.editingSubcategoryName = ""
        root.editingSubcategoryDraftName = ""
    }

    function startEdit(subcategoryId, subcategoryName, isDefault) {
        if (isDefault) {
            root.errorMessage = "默认二级分类不能改名"
            return
        }

        root.errorMessage = ""
        root.editingSubcategoryId = subcategoryId
        root.editingSubcategoryName = subcategoryName
        root.editingSubcategoryDraftName = subcategoryName
    }

    function submitEdit() {
        if (root.editingSubcategoryId <= 0) {
            return
        }

        const subcategoryName = root.editingSubcategoryDraftName.trim()
        if (subcategoryName.length === 0) {
            root.errorMessage = "二级分类名称不能为空"
            return
        }

        const result = appController.updateSubcategory(root.editingSubcategoryId, subcategoryName)
        if (!result.success) {
            root.errorMessage = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "更新二级分类失败"
            return
        }

        root.errorMessage = ""
        root.cancelEdit()
        root.refreshSubcategories()
    }

    Component.onCompleted: refreshSubcategories()
    onCategoryIdChanged: {
        root.cancelEdit()
        refreshSubcategories()
    }

    ListView {
        id: subcategoryListView

        anchors.fill: parent
        anchors.margins: root.pageMargin
        anchors.bottomMargin: root.bottomInset
        spacing: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: appController.subcategoryListModel

        header: Column {
            width: subcategoryListView.width
            spacing: 16

            Item {
                width: parent.width
                height: 48

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - backButton.width - 12
                    text: "二级分类管理"
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
                height: root.errorMessage.length > 0 ? 92 : 68
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Text {
                        width: parent.width
                        text: "当前一级分类"
                        color: "#5f6368"
                        font.pixelSize: 13
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: root.categoryName.length > 0 ? root.categoryName : "未选择分类"
                        color: "#202124"
                        font.pixelSize: 18
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: root.errorMessage
                        color: "#b3261e"
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        visible: root.errorMessage.length > 0
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 108
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Text {
                        width: parent.width
                        text: "新增二级分类"
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
                            border.color: subcategoryNameInput.activeFocus ? "#1a73e8" : "#dadce0"

                            TextInput {
                                id: subcategoryNameInput

                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                                color: "#202124"
                                font.pixelSize: 16
                                text: root.newSubcategoryName
                                onTextChanged: {
                                    if (root.newSubcategoryName !== text) {
                                        root.newSubcategoryName = text
                                    }
                                }
                                onAccepted: root.submitSubcategory()
                            }

                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 12
                                anchors.verticalCenter: parent.verticalCenter
                                text: "输入二级分类名称"
                                color: "#9aa0a6"
                                font.pixelSize: 16
                                visible: subcategoryNameInput.text.length === 0
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
                                onClicked: root.submitSubcategory()
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: root.editingSubcategoryId > 0 ? 108 : 0
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"
                visible: root.editingSubcategoryId > 0

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Text {
                        width: parent.width
                        text: "编辑二级分类"
                        color: "#202124"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    Row {
                        width: parent.width
                        height: 44
                        spacing: 10

                        Rectangle {
                            width: parent.width - saveEditButton.width - cancelEditButton.width - parent.spacing * 2
                            height: parent.height
                            radius: 8
                            color: "#ffffff"
                            border.color: editNameInput.activeFocus ? "#1a73e8" : "#dadce0"

                            TextInput {
                                id: editNameInput

                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                verticalAlignment: TextInput.AlignVCenter
                                clip: true
                                color: "#202124"
                                font.pixelSize: 16
                                text: root.editingSubcategoryDraftName
                                onTextChanged: {
                                    if (root.editingSubcategoryDraftName !== text) {
                                        root.editingSubcategoryDraftName = text
                                    }
                                }
                                onAccepted: root.submitEdit()
                            }
                        }

                        Rectangle {
                            id: saveEditButton

                            width: 64
                            height: parent.height
                            radius: 8
                            color: saveEditMouseArea.pressed ? "#185abc" : "#1a73e8"

                            Text {
                                anchors.centerIn: parent
                                text: "保存"
                                color: "#ffffff"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            MouseArea {
                                id: saveEditMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.submitEdit()
                            }
                        }

                        Rectangle {
                            id: cancelEditButton

                            width: 64
                            height: parent.height
                            radius: 8
                            color: cancelEditMouseArea.pressed ? "#f1f3f4" : "#ffffff"
                            border.color: "#dadce0"

                            Text {
                                anchors.centerIn: parent
                                text: "取消"
                                color: "#3c4043"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            MouseArea {
                                id: cancelEditMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.cancelEdit()
                            }
                        }
                    }
                }
            }

            Item {
                width: parent.width
                height: 2
            }
        }

        delegate: Rectangle {
            required property string name
            required property bool isDefault

            width: subcategoryListView.width
            height: 64
            radius: 8
            color: "#ffffff"
            border.color: "#dadce0"

            Row {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 12
                spacing: 12

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - editButton.width - parent.spacing
                    spacing: 4

                    Text {
                        width: parent.width
                        text: name
                        color: "#202124"
                        font.pixelSize: 16
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: isDefault ? "默认二级分类" : "自定义二级分类"
                        color: isDefault ? "#5f6368" : "#1a73e8"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    id: editButton

                    anchors.verticalCenter: parent.verticalCenter
                    width: 56
                    height: 32
                    radius: 6
                    visible: !isDefault
                    color: editArea.pressed ? "#d2e3fc" : "#e8f0fe"
                    border.color: "#1a73e8"

                    Text {
                        anchors.centerIn: parent
                        text: "编辑"
                        color: "#174ea6"
                        font.pixelSize: 14
                    }

                    MouseArea {
                        id: editArea

                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.startEdit(model.id, name, isDefault)
                    }
                }
            }
        }

        footer: Item {
            width: subcategoryListView.width
            height: root.bottomInset
        }
    }

    Text {
        anchors.centerIn: parent
        width: Math.max(0, parent.width - root.pageMargin * 2)
        text: "暂无二级分类"
        color: "#5f6368"
        font.pixelSize: 18
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        visible: subcategoryListView.count === 0 && root.errorMessage.length === 0
    }
}
