const util = require('./Util.js');
const database = require('./Database.js');

// ----- ----- DATA PARSER ----- -----
var parseData = function(data){
	var jsonData;
	try {
		jsonData = JSON.parse(data.toString());
	} catch (e) {
		util.consoleDisplay("Error when parsing '" + str + "' to JSON : " + e.message, "warn");
		return;
	}
	
	// check data format
	if(!chackData(jsonData)){
		util.consoleDisplay("Data '" + str + "' is in an invalid format", "warn");
		return;
	}
	return jsonData;
}

var checkData = function(json){
	// data is here + data is array + data length != 0
	if(!json.data || !Array.isArray(json.data) || json.data.length == 0)
		return false;
	for(var i in json.data){
		// sensor is here + sensor isn't object + sensorTypes is here + sensorTypes is array + sensorTypes length != 0
		if(!json.data[i].sensor || typeof json.sensor === 'object' || !json.data[i].sensorTypes
				|| typeof json.data[i].sensorTypes === 'object' || !Array.isArray(json.data[i].sensorTypes)
				|| json.data[i].sensorTypes.length == 0)
			return false;
	for(var j in json.data[i].sensorTypes){
			// sensorType is here + sensorType isn't object + data is here + data is array + data length != 0
			// + unit is here + unit is array + unit length != 0 + unit and data have the same length
			if(!json.data[i].sensorTypes[j].sensorType || typeof json.data[i].sensorTypes[j].sensorType === 'object'
					|| !json.data[i].sensorTypes[j].data || !Array.isArray(json.data[i].sensorTypes[j].data)
					|| json.data[i].sensorTypes[j].data.length == 0 || !json.data[i].sensorTypes[j].unit
					|| !Array.isArray(json.data[i].sensorTypes[j].unit) || json.data[i].sensorTypes[j].unit.length == 0
					|| json.data[i].sensorTypes[j].data.length != json.data[i].sensorTypes[j].unit.length)
				return false;
			for(var k in json.data[i].sensorTypes[j].data){
				// data aren't object
				if(typeof json.data[i].sensorTypes[j].data[k] === 'object')
					return false;
			}
			for(var k in json.data[i].sensorTypes[j].units){
				// units aren't object
				if(typeof json.data[i].sensorTypes[j].units[k] === 'object')
					return false;
			}
		}
	}
	return true;
}

var checkDataType = function(json){
	// _id is here + is isn't object + desc is here + desc isn't object + fieldNames is here + fieldNames is array
	// + fieldNames length != 0
	if(!json._id || typeof json._id === 'object' || !json.desc || typeof json.desc === 'object' || !json.fieldNames 
			|| !Array.isArray(json.fieldNames) || json.fieldNames.length == 0)
		return false;
	for(var i in json.fieldNames){
		// fieldNames aren't objects
		if(typeof json.fieldNames[i] === 'object')
			return false;
	}
	return true;
}

var checkSensor = function(json){
	// _id is here + id isn't object + desc is here + desc isn't object + dataTypes is here + dataTypes is array
	// + dataTypes length != 0
	if(!json._id || typeof json._id === 'object' || !json.desc || typeof json.desc === 'object' || !json.dataTypes
			|| !Array.isArray(json.dataTypes) || json.dataTypes.length == 0)
		return false;
	for(var i in json.dataTypes){
		// dataTypes aren't objects
		if(typeof json.dataTypes[i] === 'object')
			return false;
	}
	return true;
}

module.exports = {
	parseData: parseData,
	checkData: checkData,
	checkDataType, checkDataType,
	checkSensor: checkSensor
}
