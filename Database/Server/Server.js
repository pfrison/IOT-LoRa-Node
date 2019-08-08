const util = require('./Util.js');
const database = require('./Database.js');

// ----- ----- SERVER ----- -----
const express = require('express');
const app = express();

var openServer = function(port){
	app.listen(port);
	
	// ----- DATA -----
	// index page (used to check connection)
	app.get('/', function (req, res) {
		res.setHeader('Access-Control-Allow-Origin', '*');
		res.status(200).end();
	});
	app.use(express.json(), function(err, req, res, next) {
		if(err instanceof SyntaxError) // json parse error
			res.status(400).json({error: "The JSON data in the request body is invalid"});
	});
	// get last 10 data
	app.get('/last10data', function (req, res) {
		res.setHeader('Access-Control-Allow-Origin', '*');
		
		database.getLast10Data(function(e, list){
			if(e)
				res.status(500).json({error: e});
			else if(!list)
				res.status(404).json({error: "No data in database"});
			else
				res.status(200).json(list);
		});
	});
	
	// ----- DATATYPE -----
	// add datatype
	app.post('/datatype', function (req, res) {
		res.setHeader('Access-Control-Allow-Origin', '*');
		var datatype = req.body;
		if(!datatype){
			res.status(400).json({error: "Query body empty"});
			return;
		}
		
		database.addDataType(datatype, function(e){
			if(e)
				res.status(500).json({error: e});
			else
				res.status(200).end();
		});
	});
	// update datatype
	app.put('/datatype', function (req, res) {
		res.setHeader('Access-Control-Allow-Origin', '*');
		var datatype = req.body;
		if(!datatype){
			res.status(400).json({error: "Query body empty"});
			return;
		}
		
		database.updateDataType(datatype, function(e, list){
			if(e)
				res.status(500).json({error: e});
			else
				res.status(200).end();
		});
	});
	// get datatype
	app.get('/datatype', function (req, res) {
		res.setHeader('Access-Control-Allow-Origin', '*');
		var datatype = req.query.datatype;
		if(!datatype){
			res.status(400).json({error: "DataType name not provided in query"});
			return;
		}
		
		database.getDataType(datatype, function(e, list){
			if(e)
				res.status(500).json({error: e});
			else
				res.status(200).json(list);
		});
	});
	
	// ----- SENSOR -----
	// add sensor
	app.post('/sensor', function (req, res) {
		res.setHeader('Access-Control-Allow-Origin', '*');
		var sensor = req.body;
		if(!sensor){
			res.status(400).json({error: "Query body empty"});
			return;
		}
		
		database.addSensor(sensor, function(e, list){
			if(e)
				res.status(500).json({error: e});
			else
				res.status(200).end();
		});
	});
	// update sensor
	app.put('/sensor', function (req, res) {
		res.setHeader('Access-Control-Allow-Origin', '*');
		var sensor = req.body;
		if(!sensor){
			res.status(400).json({error: "Query body empty"});
			return;
		}
		
		database.updateSensor(sensor, function(e, list){
			if(e)
				res.status(500).json({error: e});
			else
				res.status(200).end();
		});
	});
	// get sensor
	app.get('/sensor', function (req, res) {
		res.setHeader('Access-Control-Allow-Origin', '*');
		var sensorList = req.query.sensorList;
		if(sensorList){
			database.getSensorList(function(e, list){
				if(e)
					res.status(500).json({error: e});
				else
					res.status(200).json(list);
			});
			return;
		}
		
		var sensor = req.query.sensor;
		if(!sensor){
			res.status(400).json({error: "Sensor name not provided in query"});
			return;
		}
		
		database.getSensor(sensor, function(e, list){
			if(e)
				res.status(500).json({error: e});
			else if(!list)
				res.status(404).json({error: "Sensor '" + sensor + "' not found"});
			else
				res.status(200).json(list);
		});
	});
	
	// ----- OTHERS -----
	// 404 error
	app.use(function(req, res){
		res.setHeader('Access-Control-Allow-Origin', '*');
		res.status(404).end();
	});
	
	util.consoleDisplay("Server running and listening to port " + port);
}

module.exports = {
	openServer: openServer
}