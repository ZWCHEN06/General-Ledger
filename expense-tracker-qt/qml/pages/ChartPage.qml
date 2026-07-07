import QtQuick

import "../components"

Item {
    id: root

    signal backRequested()

    property var monthLabels: []
    property var incomeValues: []
    property var expenseValues: []
    property var pieItems: []
    property string trendErrorMessage: ""
    property string pieErrorMessage: ""
    readonly property int pageMargin: Math.max(16, Math.min(24, Math.round(width * 0.05)))
    readonly property int bottomInset: Qt.platform.os === "android" ? 72 : pageMargin

    function errorMessageFromResult(result, fallbackMessage) {
        if (result && result.errorMessage && result.errorMessage.length > 0) {
            return result.errorMessage
        }

        return fallbackMessage
    }

    function loadTrendData() {
        const result = appController.monthlyTrendData(6)
        if (!result || !result.success) {
            root.trendErrorMessage = root.errorMessageFromResult(result, "加载近 6 个月收支趋势失败")
            root.monthLabels = []
            root.incomeValues = []
            root.expenseValues = []
            return
        }

        root.trendErrorMessage = ""
        root.monthLabels = result.months || []
        root.incomeValues = result.income || []
        root.expenseValues = result.expense || []
    }

    function loadPieData() {
        const result = appController.categoryPieData()
        if (!result || !result.success) {
            root.pieErrorMessage = root.errorMessageFromResult(result, "加载本月支出分类占比失败")
            root.pieItems = []
            return
        }

        root.pieErrorMessage = ""
        root.pieItems = result.items || []
    }

    function loadChartData() {
        loadTrendData()
        loadPieData()
    }

    Component.onCompleted: loadChartData()

    Rectangle {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 64
        color: "#ffffff"
        z: 2

        Rectangle {
            id: backButton

            anchors.left: parent.left
            anchors.leftMargin: root.pageMargin
            anchors.verticalCenter: parent.verticalCenter
            width: 64
            height: 36
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
            width: Math.max(0, parent.width - root.pageMargin * 2 - backButton.width * 2)
            text: "图表统计"
            color: "#202124"
            font.pixelSize: 22
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: "#e0e0e0"
        }
    }

    Flickable {
        id: flickable

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        contentWidth: width
        contentHeight: contentColumn.height + root.bottomInset
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: contentColumn

            width: flickable.width
            spacing: 18
            padding: root.pageMargin

            Column {
                width: parent.width - parent.padding * 2
                spacing: 10

                Text {
                    width: parent.width
                    text: "近 6 个月收支趋势"
                    color: "#202124"
                    font.pixelSize: 20
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: root.trendErrorMessage
                    color: "#b3261e"
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    visible: root.trendErrorMessage.length > 0
                }

                BarChart {
                    width: parent.width
                    height: implicitHeight
                    monthLabels: root.monthLabels
                    incomeValues: root.incomeValues
                    expenseValues: root.expenseValues
                    visible: root.trendErrorMessage.length === 0
                }
            }

            Column {
                width: parent.width - parent.padding * 2
                spacing: 10

                Text {
                    width: parent.width
                    text: "本月支出分类占比"
                    color: "#202124"
                    font.pixelSize: 20
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: root.pieErrorMessage
                    color: "#b3261e"
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    visible: root.pieErrorMessage.length > 0
                }

                PieChart {
                    width: parent.width
                    height: implicitHeight
                    pieItems: root.pieItems
                    visible: root.pieErrorMessage.length === 0
                }
            }
        }
    }
}
