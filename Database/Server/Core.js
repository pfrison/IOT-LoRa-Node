const util = require('./Util.js');
const serialCom = require('./SerialCom.js');
const server = require('./Server.js');
const database = require('./Database.js');

// ----- ----- CORE ----- -----
try{
	database.initDatabase("mongodb://127.0.0.1:27017/", "IOT-LoRa");
	//serialCom.openSerial();
	server.openServer(8080);
} catch (e) {
	util.consoleDisplay("Initialization error : " + e.message, "err");
	process.exit(1);
}