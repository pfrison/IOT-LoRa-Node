// ----- UTIL -----
function findGetParameter(parameterName) {
    var result = null,
        tmp = [];
    location.search
        .substr(1)
        .split("&")
        .forEach(function (item) {
          tmp = item.split("=");
          if (tmp[0] === parameterName) result = decodeURIComponent(tmp[1]);
        });
    return result;
}

function sendGET(page, args, callback){
	var address = findGetParameter("address");
	if(!address)
		address = "http://127.0.0.1:8080";
	address += page;
	if(args){
		address += "?";
		var i;
		for(i = 0; i < args.length; i += 2){
			if(i == args.length - 2)
				address += args[i] + "=" + args[i+1];
			else
				address += args[i] + "=" + args[i+1] + "&";
		}
	}
	
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET", address, true); // true for asynchronous request
	xmlHttp.onload = function (e) {
		if (xmlHttp.readyState === 4){
			if(xmlHttp.responseText === ""){
				callback("");
				return;
			}
			var json;
			try{json = JSON.parse(xmlHttp.responseText);}
			catch(e){json = "";}
			callback(json);
		}else
			callback(null);
	};
	xmlHttp.onerror = function (e) {
		callback(null);
	};
	xmlHttp.send(null);
}