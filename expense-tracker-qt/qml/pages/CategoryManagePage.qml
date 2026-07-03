pragma ComponentBehavior: Bound

import QtQuick
import "../components"

Item {
    id: root

    signal backRequested()
    signal subcategoryManageRequested(int categoryId, string categoryName)

    property string selectedType: "expense"
    property string errorMessage: ""
    property string newCategoryName: ""
    property int editingCategoryId: -1
    property string editingCategoryName: ""
    property string editingCategoryDraftName: ""
    property int deletingCategoryId: -1
    property string deletingCategoryName: ""
    property bool deleteConfirmVisible: false
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function refreshCategories() {
        categoryListModel.refresh(root.selectedType)
    }

    function cancelEdit() {
        root.editingCategoryId = -1
        root.editingCategoryName = ""
        root.editingCategoryDraftName = ""
    }

    function cancelDelete() {
        root.deletingCategoryId = -1
        root.deletingCategoryName = ""
        root.deleteConfirmVisible = false
    }

    function requestDelete(categoryId, categoryName, isDefault) {
        if (isDefault) {
            root.errorMessage = "默认分类不能删除"
            return
        }

        root.errorMessage = ""
        root.deletingCategoryId = categoryId
        root.deletingCategoryName = categoryName
        root.deleteConfirmVisible = true
    }

    function confirmDelete() {
        if (root.deletingCategoryId <= 0) {
            root.cancelDelete()
            return
        }

        const result = appController.deleteCategory(root.deletingCategoryId)
        if (!result.success) {
            root.errorMessage = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "删除分类失败"
            root.cancelDelete()
            return
        }

        if (root.editingCategoryId === root.deletingCategoryId) {
            root.cancelEdit()
        }

        root.errorMessage = ""
        root.cancelDelete()
        root.refreshCategories()
    }

    function startEdit(categoryId, categoryName, isDefault) {
        if (isDefault) {
            root.errorMessage = "默认分类不能改名"
            return
        }

        root.errorMessage = ""
        root.editingCategoryId = categoryId
        root.editingCategoryName = categoryName
        root.editingCategoryDraftName = categoryName
    }

    function submitCategory() {
        const categoryName = root.newCategoryName.trim()
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
        root.newCategoryName = ""
        root.refreshCategories()
    }

    function submitEdit() {
        if (root.editingCategoryId <= 0) {
            return
        }

        const categoryName = root.editingCategoryDraftName.trim()
        if (categoryName.length === 0) {
            root.errorMessage = "分类名称不能为空"
            return
        }

        const result = appController.updateCategory(root.editingCategoryId, categoryName)
        if (!result.success) {
            root.errorMessage = result.errorMessage.length > 0
                    ? result.errorMessage
                    : "更新分类失败"
            return
        }

        root.errorMessage = ""
        root.cancelEdit()
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
                        root.cancelEdit()
                        root.cancelDelete()
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
                        root.cancelEdit()
                        root.cancelDelete()
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
                                text: root.newCategoryName
                                onTextChanged: {
                                    if (root.newCategoryName !== text) {
                                        root.newCategoryName = text
                                    }
                                }
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

            Rectangle {
                width: parent.width
                height: root.editingCategoryId > 0 ? 108 : 0
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"
                visible: root.editingCategoryId > 0

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Text {
                        width: parent.width
                        text: "编辑分类"
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
                                text: root.editingCategoryDraftName
                                onTextChanged: {
                                    if (root.editingCategoryDraftName !== text) {
                                        root.editingCategoryDraftName = text
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

        delegate: CategoryItem {
            width: categoryListView.width
            onSubcategoryManageRequested: function(categoryId, name) {
                root.subcategoryManageRequested(categoryId, name)
            }
            onEditRequested: function(categoryId, name) {
                root.startEdit(categoryId, name, isDefault)
            }
            onDeleteRequested: function(categoryId, name) {
                root.requestDelete(categoryId, name, isDefault)
            }
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

    Rectangle {
        anchors.fill: parent
        z: 10
        color: "#80000000"
        visible: root.deleteConfirmVisible

        MouseArea {
            anchors.fill: parent
            onClicked: root.cancelDelete()
        }

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width - root.pageMargin * 2, 320)
            height: 184
            radius: 8
            color: "#ffffff"
            border.color: "#dadce0"

            MouseArea {
                anchors.fill: parent
            }

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                Text {
                    width: parent.width
                    text: "确认删除分类"
                    color: "#202124"
                    font.pixelSize: 20
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: "确定要删除“" + root.deletingCategoryName + "”吗？"
                    color: "#5f6368"
                    font.pixelSize: 15
                    wrapMode: Text.WordWrap
                }

                Item {
                    width: parent.width
                    height: 12
                }

                Row {
                    width: parent.width
                    height: 42
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
                            onClicked: root.cancelDelete()
                        }
                    }

                    Rectangle {
                        width: (parent.width - parent.spacing) / 2
                        height: parent.height
                        radius: 8
                        color: confirmDeleteMouseArea.pressed ? "#a50e0e" : "#d93025"

                        Text {
                            anchors.centerIn: parent
                            text: "删除"
                            color: "#ffffff"
                            font.pixelSize: 16
                            font.bold: true
                        }

                        MouseArea {
                            id: confirmDeleteMouseArea

                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.confirmDelete()
                        }
                    }
                }
            }
        }
    }
}
