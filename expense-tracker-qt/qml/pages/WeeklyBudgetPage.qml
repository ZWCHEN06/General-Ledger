import QtQuick

Item {
    id: root

    signal backRequested()

    property date selectedWeekStartDate: getWeekStartDate(new Date())
    property string errorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function pad2(value) {
        return value < 10 ? "0" + value : String(value)
    }

    function formatDate(date) {
        return date.getFullYear() + "-" + pad2(date.getMonth() + 1) + "-" + pad2(date.getDate())
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

    function loadWeeklyBudget() {
        const weekStartDate = formatDate(root.selectedWeekStartDate)
        const result = appController.loadWeeklyBudget(weekStartDate)
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
        root.loadWeeklyBudget()
    }

    function goNextWeek() {
        root.selectedWeekStartDate = addDays(root.selectedWeekStartDate, 7)
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

                        Rectangle {
                            id: previousWeekButton

                            width: (parent.width - parent.spacing) / 2
                            height: parent.height
                            radius: 8
                            color: previousWeekMouseArea.pressed ? "#f1f3f4" : "#ffffff"
                            border.color: "#dadce0"

                            Text {
                                anchors.centerIn: parent
                                text: "上一周"
                                color: "#3c4043"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            MouseArea {
                                id: previousWeekMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.goPreviousWeek()
                            }
                        }

                        Rectangle {
                            id: nextWeekButton

                            width: (parent.width - parent.spacing) / 2
                            height: parent.height
                            radius: 8
                            color: nextWeekMouseArea.pressed ? "#f1f3f4" : "#ffffff"
                            border.color: "#dadce0"

                            Text {
                                anchors.centerIn: parent
                                text: "下一周"
                                color: "#3c4043"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            MouseArea {
                                id: nextWeekMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.goNextWeek()
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 180
                radius: 8
                color: "#ffffff"
                border.color: "#dadce0"

                Column {
                    anchors.centerIn: parent
                    width: parent.width - 32
                    spacing: 10

                    Text {
                        width: parent.width
                        text: root.errorMessage.length > 0 ? root.errorMessage : "每周预算已加载"
                        color: root.errorMessage.length > 0 ? "#b3261e" : "#202124"
                        font.pixelSize: 18
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        width: parent.width
                        text: root.errorMessage.length > 0
                                ? "请返回后重试，或检查数据库初始化状态。"
                                : "后续会在这里显示支出分类的预算、实际支出和剩余金额。"
                        color: "#5f6368"
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Item {
                width: parent.width
                height: root.bottomInset
            }
        }
    }
}
