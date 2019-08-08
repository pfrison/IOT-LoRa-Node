// ----- STYLES -----
var favorite_style = localStorage.getItem("style");
function changeStyle(styleNum, prevStyleNum){
	// style sheet
	var head = document.getElementsByTagName('HEAD')[0];
	head.querySelector("#stylesheet").href = "styles/styles" + styleNum + ".css";
	
	// page icon
	head.querySelector("#pageicon").href = "imgs/iconStyle" + styleNum + ".png";
	
	// loading screen style
	if(prevStyleNum !== undefined){
		document.getElementById("loading_screen_" + prevStyleNum).style = "display: none;";
		const prevLoadingTxt = document.getElementById("loading_screen_" + prevStyleNum).querySelector("#loading_text").innerHTML;
		document.getElementById("loading_screen_" + styleNum).querySelector("#loading_text").innerHTML = prevLoadingTxt;
	}
	document.getElementById("loading_screen_" + styleNum).style = "display: block;";
	
	// polytech + PTIT logos
	document.getElementById("polytechLogo").src = "imgs/polytechStyle" + styleNum + ".png";
	document.getElementById("PTITLogo").src = "imgs/PTITStyle" + styleNum + ".png";
	
	// save fav style
	localStorage.setItem("style", styleNum);
	favorite_style = styleNum;
}
// load fav style
setTimeout(function(){
	if(favorite_style !== undefined)
		changeStyle(favorite_style);
}, 50);