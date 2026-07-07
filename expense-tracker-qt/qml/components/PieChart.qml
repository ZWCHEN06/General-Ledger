import QtQuick

Item {
    id: root

    property var pieItems: []
    property real holeRatio: 0.4
    property color textColor: "#333333"
    property color emptyTextColor: "#999999"
    property color backgroundColor: "#FFFFFF"
    property var colorPalette: ["#4285F4","#EA4335","#FBBC04","#34A853","#FF6D01","#9334E6","#00BCD4","#E91E63","#3F51B5","#795548"]

    implicitHeight: preferredHeight()

    function itemAmount(index) {
        if (!pieItems || index < 0 || index >= pieItems.length) {
            return 0
        }

        const amount = Number(pieItems[index].amount)
        return isNaN(amount) ? 0 : Math.max(0, amount)
    }

    function itemCategory(index) {
        if (!pieItems || index < 0 || index >= pieItems.length) {
            return ""
        }

        return String(pieItems[index].category || "")
    }

    function totalAmount() {
        let total = 0
        const count = pieItems ? pieItems.length : 0

        for (let index = 0; index < count; ++index) {
            total += itemAmount(index)
        }

        return total
    }

    function visibleItemCount() {
        let count = 0
        const itemCount = pieItems ? pieItems.length : 0

        for (let index = 0; index < itemCount; ++index) {
            if (itemAmount(index) > 0) {
                ++count
            }
        }

        return count
    }

    function legendColumnCount() {
        const availableWidth = Math.max(1, width - 32)
        return Math.max(1, Math.floor(availableWidth / 150))
    }

    function legendRowCount() {
        return Math.max(1, Math.ceil(visibleItemCount() / legendColumnCount()))
    }

    function preferredHeight() {
        if (!pieItems || pieItems.length === 0 || totalAmount() <= 0) {
            return 280
        }

        return Math.max(280, 208 + legendRowCount() * 24)
    }

    function formatAmount(value) {
        if (value >= 10000) {
            return (value / 10000).toFixed(1) + "万"
        }

        return Math.round(value).toString()
    }

    function formatPercent(amount, total) {
        if (total <= 0) {
            return "0.0%"
        }

        return (amount * 100 / total).toFixed(1) + "%"
    }

    onPieItemsChanged: chartCanvas.requestPaint()
    onWidthChanged: chartCanvas.requestPaint()
    onHeightChanged: chartCanvas.requestPaint()

    Canvas {
        id: chartCanvas

        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            ctx.fillStyle = root.backgroundColor
            ctx.fillRect(0, 0, width, height)

            const total = root.totalAmount()
            if (!pieItems || pieItems.length === 0 || total <= 0) {
                ctx.fillStyle = root.emptyTextColor
                ctx.font = "14px sans-serif"
                ctx.textAlign = "center"
                ctx.textBaseline = "middle"
                ctx.fillText("本月无支出", width / 2, height / 2)
                return
            }

            const topPadding = 14
            const sidePadding = 16
            const legendTopGap = 18
            const legendLineHeight = 24
            const legendRows = root.legendRowCount()
            const legendHeight = Math.max(legendLineHeight, legendRows * legendLineHeight)
            const chartAreaHeight = Math.max(120, height - topPadding - legendTopGap - legendHeight - 8)
            const centerX = width / 2
            const centerY = topPadding + chartAreaHeight / 2
            const radius = Math.max(36, Math.min(width - sidePadding * 2, chartAreaHeight) / 2)
            const innerRadius = radius * Math.max(0, Math.min(0.9, root.holeRatio))
            let startAngle = -Math.PI / 2

            for (let index = 0; index < pieItems.length; ++index) {
                const amount = root.itemAmount(index)
                if (amount <= 0) {
                    continue
                }

                const sliceAngle = Math.PI * 2 * amount / total
                const endAngle = startAngle + sliceAngle

                ctx.beginPath()
                ctx.moveTo(centerX, centerY)
                ctx.arc(centerX, centerY, radius, startAngle, endAngle, false)
                ctx.closePath()
                ctx.fillStyle = root.colorPalette[index % root.colorPalette.length]
                ctx.fill()

                startAngle = endAngle
            }

            ctx.beginPath()
            ctx.arc(centerX, centerY, innerRadius, 0, Math.PI * 2, false)
            ctx.fillStyle = root.backgroundColor
            ctx.fill()

            ctx.fillStyle = root.textColor
            ctx.textAlign = "center"
            ctx.textBaseline = "middle"
            ctx.font = "12px sans-serif"
            ctx.fillText("总支出", centerX, centerY - 10)
            ctx.font = "bold 16px sans-serif"
            ctx.fillText(root.formatAmount(total), centerX, centerY + 12)

            const legendAreaTop = topPadding + chartAreaHeight + legendTopGap
            const columns = root.legendColumnCount()
            const columnWidth = Math.max(120, (width - sidePadding * 2) / columns)

            ctx.textAlign = "left"
            ctx.textBaseline = "middle"
            ctx.font = "11px sans-serif"

            let visibleIndex = 0
            for (let index = 0; index < pieItems.length; ++index) {
                const amount = root.itemAmount(index)
                if (amount <= 0) {
                    continue
                }

                const row = Math.floor(visibleIndex / columns)
                const column = visibleIndex % columns
                const x = sidePadding + column * columnWidth
                const y = legendAreaTop + row * legendLineHeight
                const category = root.itemCategory(index)
                const label = category + " " + root.formatAmount(amount) + " " + root.formatPercent(amount, total)

                ctx.fillStyle = root.colorPalette[index % root.colorPalette.length]
                ctx.fillRect(x, y - 5, 10, 10)

                ctx.fillStyle = root.textColor
                ctx.fillText(label, x + 16, y, Math.max(40, columnWidth - 20))
                ++visibleIndex
            }
        }
    }
}
