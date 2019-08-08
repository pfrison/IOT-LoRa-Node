const util = require('./Util.js');

// ----- ----- SERIAL COM ----- -----
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const nucleoSerialNumber = "0672FF505055877267192037";

var connectToCOM = function(comName){
	if(comName === "" || comName === undefined){
		util.consoleDisplay("Invalid port name.", "err");
		process.exit(1);
	}
	const port = new SerialPort(comName, { baudRate: 9600 });
	port.on('error', function(err) {
		util.consoleDisplay("Serial '" + comName + "' error : " + err.message, "err");
	});
	port.on('open', function(err) {
		if (err) {
			util.consoleDisplay("Can't open serial port '" + comName + "' : " + err.message, "err");
			process.exit(1);
		}
		util.consoleDisplay("Connection established to port '" + comName + "'. Reading data...");
	});
	const parserLine = port.pipe(new Readline({delimiter: '\n\r'}));
	parserLine.on('data', function(data){
		
	});
};

var openSerial = function(){
	SerialPort.list().then(function(result){
		// no devices
		if(result.length == 0){
			util.consoleDisplay("No serial port detected.", "err");
			process.exit(1);
		}
		
		// auto detect by serial number
		for(var i in result){
			if(result[i].serialNumber === nucleoSerialNumber){
				// init serial port
				connectToCOM(result[i].comName);
				return;
			}
		}
		util.consoleDisplay("Nucleo not recognized automatically.", "warn");
		
		// only one device ? -> try to connect
		if(result.length == 1){
			util.consoleDisplay("Connecting to the only device available : '" + result[0].comName + "'.");
			connectToCOM(result[0].comName);
			return;
		}
		
		// ask user
		util.consoleDisplay("Multiples devices available :");
		for(var i in result)
			util.consoleDisplay("Device '" + result[i].comName + "', ID '" + result[i].serialNumber + "'.");
		util.consoleDisplay("Please input the COM name...", "ask");
		util.askUser(function(result){
			connectToCOM(result);
		});
	}, function(err){
		util.consoleDisplay("Can't load serial ports : " + err.message, "err");
		process.exit(1);
	}).catch(function(e){
		util.consoleDisplay("Can't initialize serial com : " + e.message, "err");
		process.exit(1);
	});
}

module.exports = {
	connectToCOM: connectToCOM,
	openSerial: openSerial
}