	/**
	MA filter
	mat3(
		1./9., 1./9., 1./9.,
		1./9., 1./9., 1./9.,
		1./9., 1./9., 1./9.
	);


	inv. MA filter -> HP filter
	
	mat3(
		-1./9., -1./9., -1./9.,
		-1./9., 8./9., -1./9.,
		-1./9., -1./9., -1./9.
		);
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

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	mat3 tmat1 = 	mat3(
		-1./9., -1./9., -1./9.,
		-1./9., 8./9., -1./9.,
		-1./9., -1./9., -1./9.
		);
	
	vec3 tex = filter(tmat1, iChannel0, vec2(uv.x, 1.-uv.y), 1./1024.);
	
	//vec3 tt = vec3(1,1,1) * 1.- (tex.x + tex.y+ tex.z)/3.;
	
	gl_FragColor = vec4(tex,1.0);
}