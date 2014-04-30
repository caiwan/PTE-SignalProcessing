/**
	MA filter
	1/9 1/9 1/9
	1/9 1/9 1/9
	1/9 1/9 1/9


	inv. MA filter -> HP filter
	-1/9 -1/9 -1/9
	-1/9 8/9 -1/9
	-1/9 -1/9 -1/9
*/
/*
	Sobel operator:
	-1 0 +1    
	-2 0 +2
	-1 0 +1

*/

	// PSF
	// Point sthread function
	// pont-szetterjedesi fv
	// 

vec3 filter(mat3 filter, sampler2D sampler, vec2 uv, float ds){
	vec3 g00 = filter[0][0] * texture2D(iChannel0, uv + vec2(-ds, -ds)).rgb;
	vec3 g01 = filter[0][1] * texture2D(iChannel0, uv + vec2(-ds, 0.0)).rgb;
	vec3 g02 = filter[0][2] * texture2D(iChannel0, uv + vec2(-ds, +ds)).rgb;
	
	vec3 g10 = filter[1][0] * texture2D(iChannel0, uv + vec2(0.0, -ds)).rgb;
	vec3 g11 = filter[1][1] * texture2D(iChannel0, uv + vec2(0.0, 0.0)).rgb;
	vec3 g12 = filter[1][2] * texture2D(iChannel0, uv + vec2(0.0, +ds)).rgb;

	vec3 g20 = filter[2][0] * texture2D(iChannel0, uv + vec2(+ds, -ds)).rgb;
	vec3 g21 = filter[2][1] * texture2D(iChannel0, uv + vec2(+ds, 0.0)).rgb;
	vec3 g22 = filter[2][2] * texture2D(iChannel0, uv + vec2(+ds, +ds)).rgb;
	
	vec3 color = g00 + g01 + g02 + g10 + g11 + g12 + g20 + g21 + g22;
	return color;
}

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
	
	return vec3(1,1,1)*length(gx + gy);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	//vec3 tex = sobel(iChannel0, vec2(uv.x, 1.-uv.y), 1./1024.);
	
	mat3 tmat1 = mat3(
			-1, 0, +1,
			-2, 0, +2,
			-1, 0, +1
		);
	
	vec3 tex = filter(tmat1, iChannel0, vec2(uv.x, 1.-uv.y), 1./512.);
	
	vec3 tt = vec3(1,1,1) * 1.- (tex.x + tex.y+ tex.z)/3.;
	
	gl_FragColor = vec4(tt,1.0);
}