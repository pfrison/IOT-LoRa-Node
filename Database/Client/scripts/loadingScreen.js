// ----- LOADING SCREEN -----
var loading_in_progress = true;
var loading_error = true;

// loading
var loading_text_anim_state = 0;
var loading_text = " STAND BY ";
var loading_text_function = function() {
	const loadingScreen = document.getElementById("loading_screen_" + favorite_style);
	// is the loading still in progress ?
	if(loading_in_progress){
		// loading text animation
		var str = loading_text;
		if(loading_text_anim_state < 3){
			for(var i=0; i<loading_text_anim_state + 1; i++)
				str = "." + str + ".";
		}else{
			for(var i=0; i<(5 - loading_text_anim_state); i++)
				str = "." + str + ".";
		}
		loading_text_anim_state++;
		if(loading_text_anim_state >= 4)
			loading_text_anim_state = 0;
		loadingScreen.querySelector("#loading_text").innerHTML = str;
		
		// repeat
		setTimeout(loading_text_function, 150);
	}else{
		if(loading_error){
			loadingScreen.querySelector("#loading_text").innerHTML = "... SYSTEM OFFLINE RETRYING ...";
			setTimeout(attemptConnection, 1000);
			setTimeout(loading_text_function, 1000);
			return;
		}else
			loadingScreen.querySelector("#loading_text").innerHTML = "SYSTEM ONLINE";
		
		// animate
		const screen = document.getElementById("loading_screen_cover");
		screen.style.animation = "none";
		screen.offsetHeight;
		screen.style.animation = "anim_fade_in 0.5s ease-out reverse";
		
		// display none the loading screen
		setTimeout(function() {
			screen.style.display = "none";
		}, 490);
	}
};
setTimeout(loading_text_function, 700);

var attemptConnection = function(){
	loading_in_progress = true;
	loading_error = true;
	
	sendGET("/", null, function(value){
		if(value !== null)
			loading_error = false;
		else
			loading_error = true;
		loading_in_progress = false;
	});
};
setTimeout(attemptConnection, 700);

function showLoadingScreenAgain(){			
	// animate
	const screen = document.getElementById("loading_screen_cover");
	screen.style.animation = "none";
	screen.offsetHeight;
	screen.style.animation = "anim_fade_in 0.5s ease-out";
	
	// make loading screen focusable
	screen.style.display = "block";
		
	document.getElementById("loading_screen_" + favorite_style).querySelector("#loading_text").innerHTML = "... SYSTEM OFFLINE RETRYING ...";
	setTimeout(attemptConnection, 1500);
	setTimeout(loading_text_function, 1500);
}