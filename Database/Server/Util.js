// ----- ----- UTIL ----- -----
const stdin = process.stdin;
var stdinOn = false;

var askUser = function(stdinCallback){
	stdin.on('data', function (data) {
		if(!stdinOn)
			return;
		const str = data.toString();
		stdinCallback(str.substring(0, str.length - 2)); // remove '\n'
		stdinOn = false;
	});
}

var consoleDisplay = function(data, type = "info"){
	if(type === "err")
		console.log("\x1b[31m\x1b[47m[ERR ]\x1b[0m %s", data); // [ERR ] in fg red + bg white
	else if(type === "warn")
		console.log("\x1b[33m\x1b[47m[WARN]\x1b[0m %s", data); // [WARN] in fg yellow + bg white
	else if(type === "ask")
		console.log("\x1b[34m\x1b[47m[ASK ]\x1b[0m %s", data); // [ASK ] in fg blue + bg white
	else // "info" or anything
		console.log("\x1b[30m\x1b[47m[INFO]\x1b[0m %s", data); // [INFO] in fg black + bg white
}

module.exports = {
	askUser: askUser,
	consoleDisplay: consoleDisplay
}