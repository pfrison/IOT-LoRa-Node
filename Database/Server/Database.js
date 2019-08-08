const util = require('./Util.js');
const parser = require('./Parser.js');

const mongoClient = require("mongodb").MongoClient;

// ----- ----- DATABASE CLIENT ----- -----
var database;
var initDatabase = function(url, databaseName) {
	mongoClient.connect(url, { useNewUrlParser: true }, function(e, client) {
		if(e){
			util.consoleDisplay("Can't connect to database at '" + url + "', is the database open ? : " + e.message, "err");
			process.exit(1);
		}
		database = client.db(databaseName);
		util.consoleDisplay("Server connected to database");
	});
}

// ----- DATA COLLECTION -----
var addData = function(data, callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	
	// parse and verify data
	var jsonData = parser.parseData(data);
	if(!jsonData)
		return;
	
	// add id in data (= date in sec in UNIX format)
	jsonData._id = Math.floor(new Date() / 1000);
	
	// add data in database
	database.collection("Data").insertOne(jsonData, function(e){
		if(e){
			util.consoleDisplay("Couldn't add data in database : " + e.message, "warn");
			callback("Unexpected database error");
		}else
			callback();
	});
}
var getLast10Data = function(callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	// request data
	database.collection("Data").find({}).sort({_id:-1}).limit(10).toArray(function(e, res){
		if(e){
			util.consoleDisplay("Couldn't get last 10 data in database : " + e.message, "warn");
			callback("Unexpected database error");
		}else
			callback(false, res);
	});
}

// ----- DATATYPE COLLECTION -----
var addDataType = function(dataType, callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	// check dataType
	if(!parser.checkDataType(dataType)){
		callback("Invalid dataType format");
		return;
	}
	// duplicate ?
	getDataType(dataType._id, function(e, res){
		if(e){
			callback(e);
			return;
		}
		if(res){ // duplicate
			callback("DataType already exist");
			return;
		}
		// insert dataType
		database.collection("DataType").insertOne(dataType, function(e, res){
			if(e){
				util.consoleDisplay("Couldn't add dataType '" + dataType._id + "' in database : " + e.message, "warn");
				callback("Unexpected database error");
			}else
				callback();
		});
	});
}
var updateDataType = function(dataType, callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	// check dataType
	if(!parser.checkDataType(dataType)){
		callback("Invalid dataType format");
		return;
	}
	// exist ?
	getDataType(dataType, function(e, res){
		if(e){
			callback(e);
			return;
		}
		if(!res){ // don't exist
			callback("DataType don't exist");
			return;
		}
		// update dataType
		database.collection("DataType").update({_id: dataType._id}, dataType, function(e, res){
			if(e){
				util.consoleDisplay("Couldn't update dataType '" + dataType._id + "' in database : " + e.message, "warn");
				callback("Unexpected database error");
			}else
				callback();
		});
	});
}
var getDataType = function(dataType, callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	// request dataType
	database.collection("DataType").findOne({_id: dataType}, function(e, res){
		if(e){
			util.consoleDisplay("Couldn't get dataType '" + dataType + "' in database : " + e.message, "warn");
			callback("Unexpected database error");
		}else
			callback(false, res);
	});
}

// ----- SENSOR COLLECTION -----
var addSensor = function(sensor, callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	// check sensor
	if(!parser.checkSensor(sensor)){
		callback("Invalid sensor format");
		return;
	}
	// duplicate ?
	getSensor(sensor._id, function(e, res){
		if(e){
			callback(e);
			return;
		}
		if(res){ // duplicate
			callback("Sensor already exist");
			return;
		}
		// insert sensor
		database.collection("Sensor").insertOne(sensor, function(e, res){
			if(e){
				util.consoleDisplay("Couldn't add sensor '" + sensor._id + "' in database : " + e.message, "warn");
				callback("Unexpected database error");
			}else
				callback();
		});
	});
}
var updateSensor = function(sensor, callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	// check sensor
	if(!parser.checkSensor(sensor)){
		callback("Invalid sensor format");
		return;
	}
	// exist ?
	geSensor(sensor, function(e, res){
		if(e){
			callback(e);
			return;
		}
		if(!res){ // don't exist
			callback("Sensor don't exist");
			return;
		}
		// update sensor
		database.collection("Sensor").update({_id: sensor._id}, sensor, function(e, res){
			if(e){
				util.consoleDisplay("Couldn't update sensor '" + sensor._id + "' in database : " + e.message, "warn");
				callback("Unexpected database error");
			}else
				callback();
		});
	});
}
var getSensor = function(sensor, callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	// request sensor
	database.collection("Sensor").findOne({_id: sensor}, function(e, res){
		if(e){
			util.consoleDisplay("Couldn't get sensor '" + sensor + "' in database : " + e.message, "warn");
			callback("Unexpected database error");
		}else
			callback(false, res);
	});
}
var getSensorList = function(callback){
	if(!database){
		util.consoleDisplay("Database not open", "warn");
		callback("Database not opened");
		return;
	}
	// request sensor
	database.collection("Sensor").find({}, {projection: {_id: 1}}).toArray(function(e, res){
		if(e){
			util.consoleDisplay("Couldn't get sensor list in database : " + e.message, "warn");
			callback("Unexpected database error");
		}else
			callback(false, res);
	});
}

module.exports = {
	initDatabase: initDatabase,
	addData: addData,
	getLast10Data: getLast10Data,
	addDataType: addDataType,
	updateDataType: updateDataType,
	getDataType: getDataType,
	addSensor: addSensor,
	updateSensor: updateSensor,
	getSensor: getSensor,
	getSensorList: getSensorList
}