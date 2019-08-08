// ----- SENSORS PAGE -----
const sensorsArticleCentent = "<h1>Sensors description</h1>"
+ "<p>"
	+ "Here you can view all sensors the node use. Pick one in the list below. This list (and all sensors description) can be modified "
	+ "on the database or by sending POST or PUT methods to the web server at the \"/sensor\" page."
+ "</p>"
+ "<select id=\"sensorsSelect\" onchange=\"sensorsSelect();\">"
	+ "<option value=\"0\"></option>"
	+ "<option value=\"1000\">Sensor list not loaded yet...</option>"
+ "</select>"
+ "<h3 id=\"sensorsId\"></h3>"
+ "<p id=\"sensorsDesc\"></p>"
+ "<div id=\"sensorDatatype\"></div>";
const sensorsSidenavContent = "<span>Sensors description</span>"
+ "<a href=\"#\" onclick=\"changeToHomePage();\">Home</a>"
+ "<a href=\"#\" onclick=\"changeToDatamapPage();\">Datamap</a>"
+ "<a href=\"#\" onclick=\"changeToAboutPage();\">About this site</a>";
function changeToSensorsPage(){
	document.getElementById("centerPanel").innerHTML = sensorsArticleCentent;
	document.getElementById("sidenav").innerHTML = sensorsSidenavContent;
	
	document.getElementById("headerHome").classList = "header_in header_text_link";
	document.getElementById("headerDatamap").classList = "header_in header_text_link";
	document.getElementById("headerSensors").classList = "header_in header_text_link_active";
	document.getElementById("headerAbout").classList = "header_in header_text_link";
	
	getSensorList();
}

var sensorSelected = 0;
var sensorList = [];

function getSensorList(){
	sensorList = [];
	sendGET("/sensor", ["sensorList", "1"], function(value){
		if(!value || value.error || !Array.isArray(value)){ // connection lost or error
			if(value && value.error)
				console.warn("Error while gathering sensor list : " + value.error);
			showLoadingScreenAgain();
			sensorSelected = 0;
			return;
		}
		var selectInner = "<option value=\"0\"></option>";
		for(var i in value){
			selectInner += "<option value=\"" + (parseInt(i) + 1) + "\">" + value[i]._id + "</option>";
			sensorList[i] = value[i]._id;
		}
		document.getElementById("sensorsSelect").innerHTML = selectInner;
		document.getElementById("sensorsSelect").value = 0;
		sensorsSelect();
	});
}

function getSensorDescription(sensorId){
	document.getElementById("sensorsId").innerHTML = sensorId;
	document.getElementById("sensorsDesc").innerHTML = "Pending description...";
	
	sendGET("/sensor", ["sensor", sensorId], function(value){
		if(!value || value.error || !value.desc || !value.dataTypes){ // connection lost or error
			if(value && value.error)
				console.warn("Error while gathering sensor description : " + error);
			showLoadingScreenAgain();
			document.getElementById("sensorsSelect").value = 0;
			sensorsSelect();
			return;
		}
		document.getElementById("sensorsDesc").innerHTML = value.desc;
		var dataTypesDiv = "<h3>Type of data taken :</h3><ul>";
		for(var i in value.dataTypes)
			dataTypesDiv += "<li>" + value.dataTypes[i] + "</li>";
		dataTypesDiv += "</ul>";
		document.getElementById("sensorDatatype").innerHTML = dataTypesDiv;
	});
}

function sensorsSelect(){
	if(document.getElementById("sensorsSelect").value == 0){
		sensorSelected = 0;
		document.getElementById("sensorsId").innerHTML = "";
		document.getElementById("sensorsDesc").innerHTML = "";
		document.getElementById("sensorDatatype").innerHTML = "";
		return;
	}
	if(sensorSelected != document.getElementById("sensorsSelect").value){
		sensorSelected = document.getElementById("sensorsSelect").value;
		if(sensorList.length != 0 && sensorSelected - 1 < sensorList.length)
			getSensorDescription(sensorList[sensorSelected - 1]);
	}
}