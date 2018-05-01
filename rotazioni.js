var acc={X:0.048828,Y:-0.986328,Z:0.050781}
var sum=Math.sqrt(acc.X*acc.X+ acc.Y*acc.Y+acc.Z*acc.Z)
if(sum<9.7 || sum >1){
	console.log("we are mooving??\ncalibration aborted")
	//return;
}
var B=Math.atan(acc.X/acc.Z)
var rot={}
var rot2={}

//rotazione intorno Y di B
rot.Y=acc.Y
rot.X=acc.X*Math.cos(B) - acc.Z*Math.sin(B)
rot.Z=acc.X*Math.sin(B) + acc.Z*Math.cos(B)

var G=Math.atan(rot.Y/rot.Z)

//rotazione intorno X di G
rot2.X=rot.X
rot2.Y=rot.Y*Math.cos(G) - rot.Z*Math.sin(G)
rot2.Z=rot.Y*Math.sin(G) + rot.Z*Math.cos(G)

rot2.X=Math.round(rot2.X*100000)/100000
rot2.Y=Math.round(rot2.Y*100000)/100000
rot2.Z=Math.round(rot2.Z*100000)/100000

console.log("matrice iniziale:" + JSON.stringify(acc))
console.log("matrice intermedia:" + JSON.stringify(rot))
console.log("matrice finale:" + JSON.stringify(rot2))
console.log("angoli rotazione: B %d° G %d°", Math.round(B/6.26*360), Math.round(G/6.28*360))
console.log("modulo accelerazione",sum)

console.log("static calibration ended")

/*
check allineamento:
(lat3-lat1)/(lat2-lat1)-(lon3-lon1)/(lon2-lon1)=0
*/