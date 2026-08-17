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
    property real currentMin: 0
    property real currentMax: fixedMax
    antialiasing: true
    onSamplesChanged: requestPaint()
    onScaleModeChanged: requestPaint()
    onScaleMultiplierChanged: requestPaint()
    onFixedMaxChanged: requestPaint()
    Timer { interval: Math.max(16,Math.round(1000/Math.max(1,graph.fps))); running: graph.visible && !graph.paused; repeat: true; onTriggered: graph.requestPaint() }
    onPaint: {
        const ctx=getContext("2d"); ctx.clearRect(0,0,width,height)
        if(!samples||samples.length===0||width<=1||height<=1)return
        let minValue=0, maxValue=fixedMax, rawMin=Number(samples[0]), rawMax=rawMin
        for(let i=1;i<samples.length;++i){ const v=Number(samples[i]); rawMin=Math.min(rawMin,v); rawMax=Math.max(rawMax,v) }
        if(scaleMode==="Auto"){
            minValue=0; maxValue=Math.max(1,rawMax*1.15)
        } else if(scaleMode==="Peak"){
            const range=Math.max(1,rawMax-rawMin); minValue=Math.max(0,rawMin-range*0.18); maxValue=rawMax+range*0.18
        }
        const multiplier=Math.max(0.1,Number(scaleMultiplier))
        maxValue=minValue+Math.max(0.0001,maxValue-minValue)*multiplier
        currentMin=minValue; currentMax=maxValue
        const span=Math.max(0.0001,maxValue-minValue), step=samples.length>1?width/(samples.length-1):width
        ctx.beginPath(); for(let i=0;i<samples.length;++i){const x=i*step, n=Math.max(0,Math.min(1,(Number(samples[i])-minValue)/span)), y=height-n*height; if(i===0)ctx.moveTo(x,y); else ctx.lineTo(x,y)}
        ctx.lineWidth=1.5; ctx.strokeStyle=lineColor; ctx.stroke(); ctx.lineTo(width,height); ctx.lineTo(0,height); ctx.closePath()
        const gradient=ctx.createLinearGradient(0,0,0,height); gradient.addColorStop(0,fillColor); gradient.addColorStop(1,"rgba(145,132,217,0.00)"); ctx.fillStyle=gradient; ctx.fill()
    }
}
