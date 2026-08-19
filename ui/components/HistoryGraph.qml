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
    property var previousSamples: []
    property string animationMode: "none"
    property real blend: 1.0

    antialiasing: true

    function frameInterval() {
        return Math.max(8, Math.round(1000 / Math.max(1, fps)))
    }

    function copySamples(source) {
        const result = []
        if (!source)
            return result
        for (let i = 0; i < source.length; ++i)
            result.push(Number(source[i]))
        return result
    }

    function beginSampleTransition(nextSamples) {
        const oldSamples = copySamples(renderSamples)
        const next = copySamples(nextSamples)
        previousSamples = oldSamples
        renderSamples = next

        if (!visible || oldSamples.length < 2 || next.length < 2) {
            animationMode = "none"
            blend = 1
            animTimer.stop()
            requestPaint()
            return
        }

        if (next.length === oldSamples.length) {
            // The history window is full. The oldest point leaves at the left
            // while the new point enters at the right. Keep the outgoing point
            // just outside/at the canvas edge so the line never exposes a gap.
            animationMode = "scroll"
        } else if (next.length === oldSamples.length + 1) {
            // While history is still growing, smoothly compress the existing
            // x positions and reveal only the new endpoint on the right.
            animationMode = "append"
        } else {
            animationMode = "none"
            blend = 1
            animTimer.stop()
            requestPaint()
            return
        }

        blend = 0
        animTimer.restart()
        requestPaint()
    }

    onSamplesChanged: {
        if (!paused)
            beginSampleTransition(samples)
    }

    onPausedChanged: {
        if (paused) {
            animTimer.stop()
            blend = 1
            requestPaint()
        } else {
            previousSamples = []
            renderSamples = copySamples(samples)
            animationMode = "none"
            blend = 1
            requestPaint()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            animTimer.stop()
            return
        }
        if (!paused) {
            previousSamples = []
            renderSamples = copySamples(samples)
            animationMode = "none"
            blend = 1
        }
        requestPaint()
    }

    onScaleModeChanged: requestPaint()
    onScaleMultiplierChanged: requestPaint()
    onFixedMaxChanged: requestPaint()
    onLineColorChanged: requestPaint()
    onFillColorChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    Component.onCompleted: {
        renderSamples = copySamples(samples)
        blend = 1
        requestPaint()
    }

    Timer {
        id: animTimer
        repeat: true
        interval: graph.frameInterval()
        onTriggered: {
            graph.blend = Math.min(1, graph.blend + interval / Math.max(1, graph.animMs))
            graph.requestPaint()
            if (graph.blend >= 1) {
                graph.animationMode = "none"
                graph.previousSamples = []
                stop()
            }
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
        const progress = Math.max(0, Math.min(1, blend))

        function yFor(value) {
            const normalized = Math.max(0, Math.min(1, (Number(value) - minValue) / span))
            return height - normalized * height
        }

        ctx.beginPath()
        let firstX = 0
        let lastX = 0

        if (animationMode === "scroll" && previousSamples.length === data.length && data.length > 1) {
            const step = width / (data.length - 1)
            const offset = (1 - progress) * step
            firstX = offset - step
            ctx.moveTo(firstX, yFor(previousSamples[0]))
            for (let i = 0; i < data.length; ++i) {
                const x = i * step + offset
                ctx.lineTo(x, yFor(data[i]))
                lastX = x
            }
        } else if (animationMode === "append" && previousSamples.length + 1 === data.length && previousSamples.length > 1) {
            const oldCount = previousSamples.length
            const oldStep = width / (oldCount - 1)
            const newStep = width / (data.length - 1)
            firstX = 0
            for (let i = 0; i < oldCount; ++i) {
                const oldX = i * oldStep
                const newX = i * newStep
                const x = oldX + (newX - oldX) * progress
                if (i === 0)
                    ctx.moveTo(x, yFor(previousSamples[i]))
                else
                    ctx.lineTo(x, yFor(previousSamples[i]))
                lastX = x
            }
            const previousLast = Number(previousSamples[oldCount - 1])
            const targetLast = Number(data[data.length - 1])
            const animatedLast = previousLast + (targetLast - previousLast) * progress
            lastX = width
            ctx.lineTo(lastX, yFor(animatedLast))
        } else {
            const step = data.length > 1 ? width / (data.length - 1) : width
            firstX = 0
            for (let i = 0; i < data.length; ++i) {
                const x = i * step
                if (i === 0)
                    ctx.moveTo(x, yFor(data[i]))
                else
                    ctx.lineTo(x, yFor(data[i]))
                lastX = x
            }
        }

        ctx.lineWidth = 1.5
        ctx.strokeStyle = lineColor
        ctx.stroke()

        ctx.lineTo(lastX, height)
        ctx.lineTo(firstX, height)
        ctx.closePath()

        const gradient = ctx.createLinearGradient(0, 0, 0, height)
        gradient.addColorStop(0, fillColor)
        gradient.addColorStop(1, "rgba(145,132,217,0.00)")
        ctx.fillStyle = gradient
        ctx.fill()
    }
}
