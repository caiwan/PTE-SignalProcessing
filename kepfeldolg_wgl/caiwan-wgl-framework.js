 /**
 * Biztosit egy mainloop visszavivast, ami tobb bongeszovel is mukodik, 
 * legalabbis elvileg.
 */
window.requestAnimFrame = (function() {
  return window.requestAnimationFrame ||
         window.webkitRequestAnimationFrame ||
         window.mozRequestAnimationFrame ||
         window.oRequestAnimationFrame ||
         window.msRequestAnimationFrame ||
         function(/* function FrameRequestCallback */ callback, /* DOMElement Element */ element) {
           window.setTimeout(callback, 1);
         };
})();

/**
 * Renderer
 * Alap renderert beallit
 */ 
 
var gl;
function Renderer(ccanvasID){
	console.log("CSNC Framwrork JS edition version 0.0.1 by Caiwan/IR 2014");
	var object = {
		
		// Mezok
		canvasID : '', 
		canvas: null, 
		gl : null,

		/**
		 * Beallitja a WGL contextet
		*/
		initWebgl: function(canvasID){
			try {
				this.canvasID = canvasID;
				this.canvas = document.getElementById(canvasID);
				
				console.log("CanvasID: "+canvasID + " : "+this.canvas);
				
				this.gl = this.canvas.getContext("experimental-webgl");
				this.gl.viewportWidth = this.canvas.width;
				this.gl.viewportHeight = this.canvas.height;
				
				console.log("Canvas : " + this.canvas.width + " x " + this.canvas.height);
				
				gl = this.gl;
			} catch (e) {
				console.log("Exception:"+e);
			}
			if (!gl) {
				alert("Could not initialize WebGL, sorry :-(");
			}
		},
		
		resizeViewport: function(){
			this.canvas = document.getElementById(this.canvasID);
			this.gl.viewportWidth = this.canvas.width;
			this.gl.viewportHeight = this.canvas.height;
			console.log("Canvas : " + this.canvas.width + " x " + this.canvas.height);			
		},
		
		/**
		* Mainloop + timer fuggveny
		*/
		FPS : 0,
		consoleLogFPS: 0,
		showFPSCallback:function(FPS){},
		
		frameCount : 0,
		frameTime : 0,
		lastTime : 0,
		currentTime : 0,
		mainloopCallback : null,
		
		mainLoop: function(){
			var _this = this;
			requestAnimFrame(function(){_this.mainLoop();});
			
			this.mainloopCallback(this.currentTime);
			var elapsed = 0;
			var timeNow = new Date().getTime();
			if (this.lastTime != 0) {
				elapsed = timeNow - this.lastTime;
				this.currentTime += elapsed;
			}
			this.lastTime = timeNow;
			
			if (this.frameTime >= 1000){
				this.FPS = this.frameCount;
				this.frameCount = 0;
				this.frameTime = 0;
				if (this.consoleLogFPS) console.log("FPS: " + this.FPS);
				this.showFPSCallback(this.FPS);
			} else {
				this.frameTime += elapsed;
				this.frameCount += 1;
			}
		}
		
	};
	
	object.initWebgl(ccanvasID);
	return object;
}

/**
 * Input
 */

 function Input(){
	return {
	
	};
 }
 
/**
  * Alap model osztaly
  * Vertex, textura, normal adatokat tartalmaz
  * + modelnezeti matrixots
  */
 
 function Model(){
	return {
	
	};
 }
 
 // texture helper stuff
 //From: http://jsperf.com/encoding-xhr-image-data/5
function arrayBufferDataUri(raw, type) {
    var base64 = '';
    var encodings = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
    var bytes = new Uint8Array(raw);
    var byteLength = bytes.byteLength;
    var byteRemainder = byteLength % 3;
    var mainLength = byteLength - byteRemainder;
    var a, b, c, d;
    var chunk;
    // Main loop deals with bytes in chunks of 3
    for (var i = 0; i < mainLength; i = i + 3) {
        // Combine the three bytes into a single integer
        chunk = (bytes[i] << 16) | (bytes[i + 1] << 8 ) | bytes[i + 2];
        // Use bitmasks to extract 6-bit segments from the triplet
        a = (chunk & 16515072) >> 18; // 16515072 = (2^6 - 1) << 18
        b = (chunk & 258048) >> 12;   // 258048   = (2^6 - 1) << 12
        c = (chunk & 4032) >> 6;      // 4032     = (2^6 - 1) << 6
        d = chunk & 63;               // 63       = 2^6 - 1
        // Convert the raw binary segments to the appropriate ASCII encoding
        base64 += encodings[a] + encodings[b] + encodings[c] + encodings[d]
    }
    // Deal with the remaining bytes and padding
    if (byteRemainder == 1) {
        chunk = bytes[mainLength];
        a = (chunk & 252) >> 2; // 252 = (2^6 - 1) << 2
        // Set the 4 least significant bits to zero
        b = (chunk & 3) << 4;   // 3   = 2^2 - 1
        base64 += encodings[a] + encodings[b] + '==';
    }
    else if (byteRemainder == 2) {
        chunk = (bytes[mainLength] << 8 ) | bytes[mainLength + 1];
        a = (chunk & 16128) >> 8; // 16128 = (2^6 - 1) << 8
        b = (chunk & 1008) >> 4;  // 1008  = (2^6 - 1) << 4
        // Set the 2 least significant bits to zero
        c = (chunk & 15) << 2;    // 15    = 2^4 - 1
        base64 += encodings[a] + encodings[b] + encodings[c] + '=';
    }
    return "data:image/"+type+";base64," + base64;
}
 
/**
 * Texture
 */
function Texture(_uri){
	var _id = gl.createTexture();
	
	var obj = {
		id: _id,
		uri: _uri,
		level: 0,
		image: 0,
		
		buildTexture: function(image){
			this.image = image;
			
			gl.bindTexture(gl.TEXTURE_2D, this.id);
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.image);
			
			// TODO: parameterezes finomitasa
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
			gl.generateMipmap(gl.TEXTURE_2D);
			
			gl.bindTexture(gl.TEXTURE_2D, null);
		},
		
		bind: function(level){
			this.level = level;
			gl.activeTexture(gl.TEXTURE0 + this.level);
			gl.bindTexture(gl.TEXTURE_2D, this.id);
		},
		
		unbind: function(){
			gl.activeTexture(gl.TEXTURE0 + level);
			gl.bindTexture(gl.TEXTURE_2D, 0);
		},
	};
	
	console.log("Loading image as texture:"+_uri+" at "+_id);
	try {
	/*
		var xhr = new XMLHttpRequest();
		xhr.open("GET", _uri, true);
		xhr.responseType = "arraybuffer";
		xhr.onload = function() {
			var img = new Image();
			img.crossOrigin = "Anonymous";
			img.onload = function() {
				obj.buildTexture(img);
				console.log("Building texture:"+_uri);
			};
			img.src = arrayBufferDataUri(xhr.response, "png");
		};
		xhr.send(null);
	*/
	
	var img = new Image();
	img.crossOrigin = "Anonymous";
	img.onload = function() {
		obj.buildTexture(img);
		console.log("Building texture:"+_uri);
	};
	img.src = _uri;
	
	} catch (ex){
		// source-orign-policy
		if(ex.code = 19){
			console.log("WARNING: Cannot load the orthodox way due to SOP. Trying an other method. id="+_uri);
			//throw ex;
			obj.id = null;
		} else {
			throw ex;
		}
	}
	
	return obj;
}

/**
 * FBO
 */
function FBO() {
	return{
	
	};
}

/**
 * Shader
 */
function Shader(vertex_id, fragment_id){
	var vertexSource = document.getElementById(vertex_id);
	var fragmentSource = document.getElementById(fragment_id);
	
	if (!vertexSource || !fragmentSource) {
		console.log("Missing shader source.");
		if(vertexSource) console.log("VERTEX OK.");
		if(fragmentSource) console.log("FRAGMENT OK.");
		return null;
	}
	
	if (vertexSource.type != "x-shader/x-vertex" || fragmentSource.type != "x-shader/x-fragment"){
		console.log("Shader source type mismatch. VSS:" + vertexSource.type + " FSS:" + fragmentSource.type);
		return null;
	}
	
	console.log("Generating new shader. VSS:"+ vertex_id + " FSS:" + fragment_id);
	
	// Vertex shader
	var vertexShader = gl.createShader(gl.VERTEX_SHADER);
	
	var str = "";
	var k = vertexSource.firstChild;
	while (k) {
		if (k.nodeType == 3) {
			str += k.textContent;
		}
		k = k.nextSibling;
	}

	gl.shaderSource(vertexShader, str);
	gl.compileShader(vertexShader);
	
	if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
		alert("Vertex shader compilation failed.");
		console.log(gl.getShaderInfoLog(vertexShader));
		return null;
	}
	
	console.log("Vertex Shader OK id=" + vertexShader);
	
	// Fragment shader
	var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	
	str = "";
	k = fragmentSource.firstChild;
	while (k) {
		if (k.nodeType == 3) {
			str += k.textContent;
		}
		k = k.nextSibling;
	}

	gl.shaderSource(fragmentShader, str);
	gl.compileShader(fragmentShader);
	
	if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
		alert("Fragment shader compilation failed.");
		console.log(gl.getShaderInfoLog(fragmentShader));
		return null;
	}
	
	console.log("Fragment Shader OK id=" + fragmentShader);
	
	// Link
	var shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

	if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
		alert("Shader linking failed.");
		console.log(gl.getProgramInfoLog(shaderProgram));
	}
	
	console.log("Shader linking OK id="+shaderProgram);
	
	return {
		program : shaderProgram,
		
		// attrib/uniform helyek
		getAttribLoc  : function(name) {return gl.getAttribLocation(this.program, name);},
		getUniformLoc : function(name) {return gl.getUniformLocation(this.program, name);},
		
		// engedelyezes/tiltas nev szerint
		enableAttribArray : function(name) {gl.enableVertexAttribArray(this.getAttribLoc(name));},
		disableAttribArray : function(name) {gl.disableVertexAttribArray(this.getAttribLoc(name));},
		
		// direkt engedelyezes/tiltas
		enableAttribArrayByID : function(loc) {gl.enableVertexAttribArray(ioc);},
		disableAttribArrayBYID : function(loc) {gl.disableVertexAttribArray(loc);},
		
		// uniformok
		setUniform1f : function(name, v0){gl.uniform1f(this.getUniformLoc(name), v0);},
		setUniform2f : function(name, v0, v1){gl.uniform2f(this.getUniformLoc(name), v0, v1);},
		setUniform3f : function(name, v0, v1, v2){gl.uniform3f(this.getUniformLoc(name), v0, v1, v2);},
		setUniform4f : function(name, v0, v1, v2, v3){gl.uniform4f(this.getUniformLoc(name), v0, v1, v2, v3);},
		setUniform3m : function(name, mat){gl.uniformMatrix3fv(this.getUniformLoc(name), false, new Float32Array(mat));},
		setUniform4m : function(name, mat){gl.uniformMatrix4fv(this.getUniformLoc(name), false, new Float32Array(mat));},
		
		// bind/unbind
		bind : function(){ gl.useProgram(this.program);},
		unbind : function (){/*gl.useProgram(0);*/},
	}
}

/**
 * Buffer object
 * @param array - JS typed array object (Float32Array, ... ) 
 * @param elemSize size of one element (1 - scalar, 2 - vec2 ... )
 * @param elemSize number of elements
 * https://developer.mozilla.org/en-US/docs/JavaScript/Typed_arrays
 */ 
function Buffer(array, elemSize, len) {
	var buffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
	
	gl.bufferData(gl.ARRAY_BUFFER, array, gl.STATIC_DRAW);
	buffer.itemSize = elemSize;
	buffer.numItems = len;
	
	return{
		id : buffer,
		elem : elemSize,
		length : len,
		type : gl.ARRAY_BUFFER,
		dataType : gl.FLOAT,
		
		bind : function(){gl.bindBuffer(this.type, this.id);},
		
		bindToAttrib : function(shader, name){
			gl.vertexAttribPointer(shader.getAttribLoc(name), this.elem, this.dataType, false, 0, 0);
		},
		
		unbind : function(){gl.bindBuffer(this.type, 0);},
	};
}