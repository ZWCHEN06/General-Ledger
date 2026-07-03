import QtQuick

Rectangle {
    id: categoryItem

    required property string name
    required property bool isDefault
    required property int categoryId

    signal editRequested(int categoryId, string name)
    signal deleteRequested(int categoryId, string name)
    signal subcategoryManageRequested(int categoryId, string name)

    implicitHeight: 64
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
            width: parent.width - actionRow.width - parent.spacing
            spacing: 4

            Text {
                width: parent.width
                text: categoryItem.name
                color: "#202124"
                font.pixelSize: 16
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: categoryItem.isDefault ? "默认分类" : "自定义分类"
                color: categoryItem.isDefault ? "#5f6368" : "#1a73e8"
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }

        Row {
            id: actionRow

            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Rectangle {
                width: 76
                height: 32
                radius: 6
                color: subcategoryArea.pressed ? "#e6f4ea" : "#ffffff"
                border.color: "#34a853"

                Text {
                    anchors.centerIn: parent
                    text: "二级分类"
                    color: "#137333"
                    font.pixelSize: 14
                }

                MouseArea {
                    id: subcategoryArea

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: categoryItem.subcategoryManageRequested(categoryItem.categoryId, categoryItem.name)
                }
            }

            Rectangle {
                width: 56
                height: 32
                radius: 6
                visible: !categoryItem.isDefault
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
                    onClicked: categoryItem.editRequested(categoryItem.categoryId, categoryItem.name)
                }
            }

            Rectangle {
                width: 56
                height: 32
                radius: 6
                visible: !categoryItem.isDefault
                color: deleteArea.pressed ? "#fad2cf" : "#fce8e6"
                border.color: "#d93025"

                Text {
                    anchors.centerIn: parent
                    text: "删除"
                    color: "#a50e0e"
                    font.pixelSize: 14
                }

                MouseArea {
                    id: deleteArea

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: categoryItem.deleteRequested(categoryItem.categoryId, categoryItem.name)
                }
            }
        }
    }
}
