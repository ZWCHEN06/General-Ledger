import QtQuick

Item {
    id: root

    property var monthLabels: []
    property var incomeValues: []
    property var expenseValues: []
    property color incomeColor: "#34A853"
    property color expenseColor: "#EA4335"
    property color textColor: "#333333"
    property color gridColor: "#DDDDDD"
    property color emptyTextColor: "#999999"
    property int barGroupGap: 20
    property int barGap: 4

    implicitHeight: 260

    function valueAt(values, index) {
        if (!values || index < 0 || index >= values.length) {
            return 0
        }

        const value = Number(values[index])
        return isNaN(value) ? 0 : Math.max(0, value)
    }

    function formatAmount(value) {
        if (value >= 10000) {
            return (value / 10000).toFixed(1) + "万"
        }

        return Math.round(value).toString()
    }

    function maxAmount() {
        let maxValue = 0
        const count = Math.max(monthLabels ? monthLabels.length : 0,
                               incomeValues ? incomeValues.length : 0,
                               expenseValues ? expenseValues.length : 0)

        for (let index = 0; index < count; ++index) {
            maxValue = Math.max(maxValue,
                                valueAt(incomeValues, index),
                                valueAt(expenseValues, index))
        }

        return maxValue
    }

    function hasData() {
        return monthLabels && monthLabels.length > 0 && maxAmount() > 0
    }

    onMonthLabelsChanged: chartCanvas.requestPaint()
    onIncomeValuesChanged: chartCanvas.requestPaint()
    onExpenseValuesChanged: chartCanvas.requestPaint()
    onWidthChanged: chartCanvas.requestPaint()
    onHeightChanged: chartCanvas.requestPaint()

    Canvas {
        id: chartCanvas

        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            ctx.textBaseline = "middle"
            ctx.textAlign = "center"

            if (!root.hasData()) {
                ctx.fillStyle = root.emptyTextColor
                ctx.font = "14px sans-serif"
                ctx.fillText("暂无数据", width / 2, height / 2)
                return
            }

            const leftPadding = 48
            const rightPadding = 18
            const topPadding = 26
            const bottomPadding = 52
            const legendHeight = 24
            const chartTop = topPadding
            const chartBottom = height - bottomPadding
            const chartLeft = leftPadding
            const chartRight = width - rightPadding
            const chartWidth = Math.max(1, chartRight - chartLeft)
            const chartHeight = Math.max(1, chartBottom - chartTop - legendHeight)
            const chartBaseY = chartTop + chartHeight
            const maxValue = root.maxAmount()
            const normalizedMax = maxValue <= 0 ? 1 : maxValue
            const tickCount = 4

            ctx.strokeStyle = root.gridColor
            ctx.lineWidth = 1
            ctx.fillStyle = root.textColor
            ctx.font = "11px sans-serif"
            ctx.textAlign = "right"

            for (let tick = 0; tick <= tickCount; ++tick) {
                const ratio = tick / tickCount
                const y = chartBaseY - chartHeight * ratio
                const tickValue = normalizedMax * ratio

                ctx.beginPath()
                ctx.moveTo(chartLeft, y)
                ctx.lineTo(chartRight, y)
                ctx.stroke()

                ctx.fillText(root.formatAmount(tickValue), chartLeft - 8, y)
            }

            const monthCount = monthLabels.length
            const availableGroupWidth = Math.max(12, (chartWidth - root.barGroupGap * Math.max(0, monthCount - 1)) / monthCount)
            const barWidth = Math.max(4, (availableGroupWidth - root.barGap) / 2)

            ctx.textAlign = "center"
            for (let index = 0; index < monthCount; ++index) {
                const groupX = chartLeft + index * (availableGroupWidth + root.barGroupGap)
                const income = root.valueAt(incomeValues, index)
                const expense = root.valueAt(expenseValues, index)
                const incomeHeight = chartHeight * income / normalizedMax
                const expenseHeight = chartHeight * expense / normalizedMax
                const incomeX = groupX
                const expenseX = groupX + barWidth + root.barGap
                const incomeY = chartBaseY - incomeHeight
                const expenseY = chartBaseY - expenseHeight

                ctx.fillStyle = root.incomeColor
                ctx.fillRect(incomeX, incomeY, barWidth, incomeHeight)

                ctx.fillStyle = root.expenseColor
                ctx.fillRect(expenseX, expenseY, barWidth, expenseHeight)

                ctx.font = "10px sans-serif"
                ctx.fillStyle = root.textColor
                if (income > 0) {
                    ctx.fillText(root.formatAmount(income), incomeX + barWidth / 2, Math.max(10, incomeY - 8))
                }
                if (expense > 0) {
                    ctx.fillText(root.formatAmount(expense), expenseX + barWidth / 2, Math.max(10, expenseY - 8))
                }

                ctx.font = "11px sans-serif"
                ctx.fillText(monthLabels[index], groupX + availableGroupWidth / 2, chartBaseY + 18)
            }

            const legendY = height - 18
            const legendItemWidth = 72
            const legendStartX = width / 2 - legendItemWidth

            ctx.font = "12px sans-serif"
            ctx.textAlign = "left"

            ctx.fillStyle = root.incomeColor
            ctx.fillRect(legendStartX, legendY - 5, 10, 10)
            ctx.fillStyle = root.textColor
            ctx.fillText("收入", legendStartX + 16, legendY)

            ctx.fillStyle = root.expenseColor
            ctx.fillRect(legendStartX + legendItemWidth, legendY - 5, 10, 10)
            ctx.fillStyle = root.textColor
            ctx.fillText("支出", legendStartX + legendItemWidth + 16, legendY)
        }
    }
}
