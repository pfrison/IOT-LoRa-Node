// ----- DATAMAP PAGE -----
const datamapArticleCentent = "<h1>Datamap</h1>"
+ "<p>In this section you can consult the last 10 data uploaded to the database</p>"
+ "</br>";
const datamapSidenavContent = "<span>Datamap</span>"
+ "<a href=\"#\" onclick=\"changeToHomePage();\">Home</a>"
+ "<a href=\"#\" onclick=\"changeToSensorsPage();\">Sensors description</a>"
+ "<a href=\"#\" onclick=\"changeToAboutPage();\">About this site</a>";
function changeToDatamapPage(){
	document.getElementById("centerPanel").innerHTML = datamapArticleCentent + generateSections();
	document.getElementById("sidenav").innerHTML = datamapSidenavContent;
	
	document.getElementById("headerHome").classList = "header_in header_text_link";
	document.getElementById("headerDatamap").classList = "header_in header_text_link_active";
	document.getElementById("headerSensors").classList = "header_in header_text_link";
	document.getElementById("headerAbout").classList = "header_in header_text_link";
	
	getDataList();
}

var dataList = [];
var sectionOpened = -1;
function getDataList(){
	dataList = [];
	sendGET("/last10data", null, function(value){
		if(!value || value.error || !Array.isArray(value)){ // connection lost or error
			if(value && value.error)
				console.warn("Error while gathering sensor list : " + value.error);
			showLoadingScreenAgain();
			sensorSelected = 0;
			return;
		}
		dataList = value;
		var i;
		for(i = 0; i < value.length; i++)
			document.getElementById("sectionA" + i).innerHTML = "\u25BC DATA FROM " + new Date(dataList[i]._id*1000).toLocaleString();
		for(; i < 10; i++)
			document.getElementById("sectionA" + i).innerHTML = "\u25BC EMPTY";
	});
}

function generateSections(){
	var sections = "";
	for(var i = 0; i < 10; i++)
		sections += "<section id=\"section" + i + "\">"
			+ "<a href=\"#\" id=\"sectionA" + i + "\" onclick=\"datamapSectionClick(" + i + ");\">\u25BC DATA NOT RECOVERED YET ...</a>"
			+ "</section>";
	return sections;
}

function datamapSectionClick(sectionId){
	if(document.getElementById("sectionOpened"))
		document.getElementById("sectionOpened").outerHTML = "";
	if(!dataList || sectionOpened == sectionId || sectionId >= dataList.length)
		sectionOpened = -1;
	else{
		var div = "<div id=\"sectionOpened\" class=\"sectionContent\">";
		for(var i in dataList[sectionId].data){
			div += "<h4>Sensor " + dataList[sectionId].data[i].sensor + "</h4>"
				+ "<ul>";
			for(var j in dataList[sectionId].data[i].sensorTypes){
				div += "<li>" + dataList[sectionId].data[i].sensorTypes[j].sensorType + " : ";
				for(var k in dataList[sectionId].data[i].sensorTypes[j].data){
					if(k != 0)
						div += ", ";
					if(dataList[sectionId].data[i].sensorTypes[j].units[k] != "")
						div += dataList[sectionId].data[i].sensorTypes[j].data[k] + " " + dataList[sectionId].data[i].sensorTypes[j].units[k];
					else
						div += dataList[sectionId].data[i].sensorTypes[j].data[k];
				}
				div += ".</li>";
			}
			div += "</ul>";
		}
		div += "</div>";
		document.getElementById("section" + sectionId).outerHTML += div;
		sectionOpened = sectionId;
	}
}