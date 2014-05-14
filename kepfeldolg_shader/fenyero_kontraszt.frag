#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;

uniform sampler2D tex0;

uniform float brightness;
uniform float contrast;
uniform float gamma;
uniform vec2 treshold;

vec3 process(vec3 _color, 
	in float _brightness, 
	in float _contrast,
	in float _gamma,
	in vec2 _treshold
){
	vec3 color = _color;
	color = color * _contrast + _brightness;
	color = clamp(color, 0., 1.);	// invertalas megakadalyozasa
	color = vec3(
		pow(color.r, _gamma),
		pow(color.g, _gamma),
		pow(color.b, _gamma)
	);
	color = clamp(color, treshold.x, treshold.y);
	return color;
}

void main()
{
	vec2 uv = gl_FragCoord.xy / resolution.y;
	
	vec3 tex = texture2D(tex0, vec2(uv.s, 1.-uv.t)).rgb;	// textura
	if (uv.x>1.) tex = vec3(uv.t);							// jobb oldali gradient
	
	// szuro
	vec3 color = process(tex, brightness, contrast, gamma, treshold);
	
	// hisztogram spektruma
	float v = process(vec3(uv.t), brightness, contrast, gamma, treshold).x / (resolution.y/(resolution.x-resolution.y));
	if(uv.x>1. && uv.x - v - 1. <= 0. ) color = .75*color + .25*vec3(.25,.75,.25);
	
	
	gl_FragColor=vec4(color, 1.0 );
}

