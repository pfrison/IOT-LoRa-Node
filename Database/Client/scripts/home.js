// ----- HOME PAGE -----
const homeArticleContent = "<h1>IOT-LoRa database interface</h1>"
+ "<p>"
	+ "The project consisted of making a multi-sensor node using LoRa, a powerful, low power and long range communication protocol. "
	+ "The node (a <a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://os.mbed.com/platforms/ST-Discovery-LRWAN1/\">DISCO-L072CZ-LRWAN1</a>) and its server "
	+ "(a <a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://www.st.com/en/evaluation-tools/nucleo-l152re.html\">NUCLEO-L152RE</a>) are "
	+ "<a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://www.mbed.com/\">mbed</a> electronic cards with "
	+ "<a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://www.st.com/en/microcontrollers-microprocessors/stm32l072cz.html\">STM32L0</a> microcontrollers."
+ "</p>"
+ "<p>"
	+ "The project is divided in 4 parts : the <b>node</b>, the <b>server</b>, the <b>database</b> and the <b>interface</b>."
+ "</p>"
+ "<h3>The node</h3>"
+ "<p>"
	+ "The node (<a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://os.mbed.com/platforms/ST-Discovery-LRWAN1/\">DISCO-L072CZ-LRWAN1</a>) "
	+ "is a development board with a LoRa emitter/receiver included. This LoRa emitter is using a "
	+ "<a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1276\">SX1276</a> "
	+ "ship to receive commands from the microcontroller and format data into LoRa frames that can later be sent to the LoRa network."
+ "</p>"
+ "<h3>The server</h3>"
+ "<p>"
	+ "The server (<a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://www.st.com/en/evaluation-tools/nucleo-l152re.html\">NUCLEO-L152RE</a>) "
	+ "is also a development board by mbed. This one however doesn't with LoRa built-in, I had to use the "
	+ "<a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://os.mbed.com/components/SX1276MB1xAS/\">SX1276MB1xAS shield</a> "
	+ "to send/receive data through the LoRa network. This shield work (almost) the same that the built-in LoRa transmitter on the node."
+ "</p>"
+ "<h3>The database</h3>"
+ "<p>"
	+ "For the database, I've used <a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://www.mongodb.com/\">MongoDB</a> a <b>no-SQL "
	+ "database</b>. MongoDB use the JSON format to store data and allow us to store those data with any predetermined format like SQL would. "
	+ "MongoDB is the most famous database for IOT applications. With MongoDB, I've used "
	+ "<a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://nodejs.org/\">node.js</a> to get the data receives by the server via serial "
	+ "communication. Node.js also open the <b>web server</b> that the interface use to request data."
+ "</p>"
+ "<h3>The interface</h3>"
+ "<p>"
	+ "The interface you looking at right now is simply a single HTML page. "
	+ "<a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://www.javascript.com/\">JavaScript</a> is used to modify the content dynamically "
	+ "to switch between pages. The node.js web server only produce JSON object, this interface transform these data into an human-readable "
	+ "text and graphics. This is to <b>reduce the amount of data transfered from the server to the web browser</b> : the web browser take care "
	+ "of the beauty while only the data go through the network."
+ "</p>"
+ "<h1>What to do here ?</h1>"
+ "<p>"
	+ "On this webinterface, on can access data from the \"datamap\" section. You can also access a description of all sensors used by the node "
	+ "in the \"sensors\" section."
+ "</p>";
const homeSidenavContent = "<span>Home</span>"
+ "<a href=\"#\" onclick=\"changeToDatamapPage();\">Datamap</a>"
+ "<a href=\"#\" onclick=\"changeToSensorsPage();\">Sensors description</a>"
+ "<a href=\"#\" onclick=\"changeToAboutPage();\">About this site</a>";
function changeToHomePage(){
	document.getElementById("centerPanel").innerHTML = homeArticleContent;
	document.getElementById("sidenav").innerHTML = homeSidenavContent;
	
	document.getElementById("headerHome").classList = "header_in header_text_link_active";
	document.getElementById("headerDatamap").classList = "header_in header_text_link";
	document.getElementById("headerSensors").classList = "header_in header_text_link";
	document.getElementById("headerAbout").classList = "header_in header_text_link";
}

// load home at startup
setTimeout(function(){
	changeToHomePage();
}, 50);