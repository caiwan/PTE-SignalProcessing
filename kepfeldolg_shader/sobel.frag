vec3 sobel(sampler2D sampler, vec2 uv, float ds){
	// x irany
	vec3 gx0a = -1. * texture2D(iChannel0, uv + vec2(-ds, -ds)).rgb;
	vec3 gx1a = -2. * texture2D(iChannel0, uv + vec2(-ds, 0.0)).rgb;
	vec3 gx2a = -1. * texture2D(iChannel0, uv + vec2(-ds, +ds)).rgb;
	
	vec3 gx0b = +1. * texture2D(iChannel0, uv + vec2(+ds, -ds)).rgb;
	vec3 gx1b = +2. * texture2D(iChannel0, uv + vec2(+ds, 0.0)).rgb;
	vec3 gx2b = +1. * texture2D(iChannel0, uv + vec2(+ds, +ds)).rgb;

	// y irany
	vec3 gy0a = -1. * texture2D(iChannel0, uv + vec2(-ds, -ds)).rgb;
	vec3 gy1a = -2. * texture2D(iChannel0, uv + vec2(0.0, -ds)).rgb;
	vec3 gy2a = -1. * texture2D(iChannel0, uv + vec2(+ds, -ds)).rgb;
	
	vec3 gy0b = +1. * texture2D(iChannel0, uv + vec2(-ds, +ds)).rgb;
	vec3 gy1b = +2. * texture2D(iChannel0, uv + vec2(0.0, +ds)).rgb;
	vec3 gy2b = +1. * texture2D(iChannel0, uv + vec2(+ds, +ds)).rgb;
	
	vec3 gx = (gx0a + gx1a + gx2a + gx0b + gx1b + gx2b);
	vec3 gy = (gy0a + gy1a + gy2a + gy0b + gy1b + gy2b);
	
	vec3 res = vec3(
		length(vec2(gx.x,gy.x)),
		length(vec2(gx.y,gy.y)),
		length(vec2(gx.z,gy.z))
	);
	
	return res;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec3 tex = sobel(iChannel0, vec2(uv.x, 1.-uv.y), 1./1024.);	
	gl_FragColor = vec4(tex,1.0);
}