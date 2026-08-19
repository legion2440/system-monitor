import QtQuick
import MoneSys

Canvas {
    id: graph
    property var samples: []
    property color lineColor: Theme.accentStep(300)
    property color fillColor: Qt.rgba(0.57,0.52,0.85,0.18)
    property real fixedMax: 100
    property string scaleMode: "Auto"
    property real scaleMultiplier: 1.0
    property int fps: 30
    property bool paused: false
    property int animMs: 320
    property real currentMin: 0
    property real currentMax: fixedMax
    property var renderSamples: []
    property real blend: 1.0

    antialiasing: true

    function frameInterval() { return Math.max(8, Math.round(1000 / Math.max(1, fps))) }

    onSamplesChanged: {
        if (paused)
            return
        renderSamples = samples
        if (!visible) {
            animTimer.stop()
            blend = 1
            return
        }
        blend = 0
        animTimer.restart()
        requestPaint()
    }

    onPausedChanged: {
        if (paused) {
            animTimer.stop()
            blend = 1
            requestPaint()
        } else {
            renderSamples = samples
            blend = 1
            requestPaint()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            animTimer.stop()
            blend = 1
        } else {
            renderSamples = samples
            blend = 1
            requestPaint()
        }
    }

    onScaleModeChanged: requestPaint()
    onScaleMultiplierChanged: requestPaint()
    onFixedMaxChanged: requestPaint()
    onLineColorChanged: requestPaint()
    onFillColorChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    Component.onCompleted: { renderSamples = samples; blend = 1; requestPaint() }

    Timer {
        id: animTimer
        repeat: true
        interval: graph.frameInterval()
        onTriggered: {
            graph.blend = Math.min(1, graph.blend + interval / Math.max(1, graph.animMs))
            graph.requestPaint()
            if (graph.blend >= 1)
                stop()
        }
    }

    onPaint: {
        const ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        const data = renderSamples
        if (!data || data.length === 0 || width <= 1 || height <= 1)
            return

        let minValue = 0
        let maxValue = fixedMax
        let rawMin = Number(data[0])
        let rawMax = rawMin
        for (let i = 1; i < data.length; ++i) {
            const v = Number(data[i])
            rawMin = Math.min(rawMin, v)
            rawMax = Math.max(rawMax, v)
        }

        if (scaleMode === "Auto") {
            minValue = 0
            maxValue = Math.max(1, rawMax * 1.15)
        } else if (scaleMode === "Peak") {
            const range = Math.max(1, rawMax - rawMin)
            minValue = Math.max(0, rawMin - range * 0.18)
            maxValue = rawMax + range * 0.18
        }

        const multiplier = Math.max(0.1, Number(scaleMultiplier))
        maxValue = minValue + Math.max(0.0001, maxValue - minValue) * multiplier
        currentMin = minValue
        currentMax = maxValue

        const span = Math.max(0.0001, maxValue - minValue)
        const step = data.length > 1 ? width / (data.length - 1) : width
        const offset = (1 - Math.max(0, Math.min(1, blend))) * step

        ctx.beginPath()
        for (let i = 0; i < data.length; ++i) {
            const x = i * step + offset
            const normalized = Math.max(0, Math.min(1, (Number(data[i]) - minValue) / span))
            const y = height - normalized * height
            if (i === 0)
                ctx.moveTo(x, y)
            else
                ctx.lineTo(x, y)
        }

        ctx.lineWidth = 1.5
        ctx.strokeStyle = lineColor
        ctx.stroke()
        ctx.lineTo(width + offset, height)
        ctx.lineTo(offset, height)
        ctx.closePath()

        const gradient = ctx.createLinearGradient(0, 0, 0, height)
        gradient.addColorStop(0, fillColor)
        gradient.addColorStop(1, "rgba(145,132,217,0.00)")
        ctx.fillStyle = gradient
        ctx.fill()
    }
}
