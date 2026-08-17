.pragma library
function bytes(value){ if(!isFinite(value)||value<0)return "—"; const units=["B","KB","MB","GB","TB","PB"]; let index=0; let scaled=Number(value); while(scaled>=1024&&index<units.length-1){scaled/=1024;++index} const decimals=index===0?0:(scaled>=100?0:scaled>=10?1:2); return scaled.toFixed(decimals)+" "+units[index] }
function rate(value){ return bytes(value)+"/s" }
function percent(value){ return Number(value||0).toFixed(1)+"%" }
function ghz(mhz){ if(!isFinite(mhz)||mhz<=0)return "—"; return (mhz/1000).toFixed(2)+" GHz" }
