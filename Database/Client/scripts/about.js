// ----- ABOUT PAGE -----
const aboutArticleCentent = "<h1>About this site</h1>"
+ "<p>"
	+ "This this have been created by <b>Pierre Frison</b>, French student at <b>Polytech Lille</b> (2019), for his internship project in Vietnam at the "
	+ "<b>Posts and Telecommunications Institute of Technology</b> (PTIT)."
+ "</p>"
+ "<p>"
	+ "This site is single html page dynamically populated by JavaScript."
+ "</p>"
+ "<h3>Change theme</h3>"
+ "<p>"
	+ "For beauty's sake, you can change the interface theme with the list below. The chosen theme will remain on the browser as long as "
	+ "the user doesn't clean his browser memory."
+ "</p>"
+ "<select id=\"themeChooser\" onchange=\"aboutSelectChange();\">"
	+ "<option value=\"0\">Dark theme (default)</option>"
	+ "<option value=\"1\">Light theme</option>"
	+ "<option value=\"2\">Vietnam theme</option>"
+ "</select>";
const aboutSidenavContent = "<span>About this site</span>"
+ "<a href=\"#\" onclick=\"changeToHomePage();\">Home</a>"
+ "<a href=\"#\" onclick=\"changeToDatamapPage();\">Datamap</a>"
+ "<a href=\"#\" onclick=\"changeToSensorsPage();\">Sensors description</a>";
function changeToAboutPage(){
	document.getElementById("centerPanel").innerHTML = aboutArticleCentent;
	document.getElementById("sidenav").innerHTML = aboutSidenavContent;
	
	document.getElementById("headerHome").classList = "header_in header_text_link";
	document.getElementById("headerDatamap").classList = "header_in header_text_link";
	document.getElementById("headerSensors").classList = "header_in header_text_link";
	document.getElementById("headerAbout").classList = "header_in header_text_link_active";
	
	document.getElementById("themeChooser").value = favorite_style;
}

function aboutSelectChange(){
	if(document.getElementById("themeChooser").value !== favorite_style)
		changeStyle(document.getElementById("themeChooser").value, favorite_style);
}