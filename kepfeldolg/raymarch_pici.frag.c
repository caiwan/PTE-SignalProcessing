#define MAX_ITERATION 64
#define MAX_DEPTH 60.
#define MIN_PREC 0.001

float U(float a, float b){
	return min(a, b);
}

float sphere(vec3 p, vec3 o, float r){
	return length(p-o) - r;
}

float map(vec3 pos, out vec3 hit){
	float s0 = sphere(pos, vec3(-11.,0.,0.), 5.);
	float s1 = sphere(pos, vec3(0.,0.,0.),5.);
	float s2 = sphere(pos, vec3(+11.,0.,0.),5.);
	
	float f = U(s0, U(s1,s2));
	
	hit = vec3(.6,.25, .25);
	//vec2 uv = vec2(dot(pos,vec3(0,1,0)), dot(pos,vec3(0,0,1))) / 5.;
	//hit = texture2D(iChannel0, uv).rgb;
	
	return f;
}

float intersect(in vec3 ro, in vec3 rd, out vec3 hit){
	float h = 0., t = 1.;
	for(int i=0; i<MAX_ITERATION; i++){
		if (abs(t) >  MAX_DEPTH || abs(t) < MIN_PREC) break;
		h = map(ro+t*rd, hit);
		t += h;
	}
	return t;
}

vec3 calcNormal( in vec3 pos )
{
	vec3 c = vec3(0.);
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    map(pos+eps.xyy, c) - map(pos-eps.xyy, c),
	    map(pos+eps.yxy, c) - map(pos-eps.yxy, c),
	    map(pos+eps.yyx, c) - map(pos-eps.yyx, c));
	return normalize(nor);
}

void main(void)
{
	vec2 uv = 2.*((gl_FragCoord.xy / iResolution.xy) - 0.5);
	uv.x *= iResolution.x / iResolution.y;
	
	vec3 color = vec3(0.);
	
	vec3 eye = vec3(0,0, 26.);
	vec3 center = vec3(0.);
	vec3 up = vec3(0.,1.,0.);
	
	vec3 cz = normalize(eye-center);
	vec3 cy = normalize(up);
	vec3 cx = normalize(cross(cy ,cz));
	
	mat3 modelview = mat3(cx,cy,cz);
	
	vec3 ray_dir = normalize(vec3(uv.x, uv.y, -2.5)) * modelview;
	vec3 ray_orign = eye;
	
	vec3 hit_record = vec3(0.);
	
	float dst = intersect(ray_orign, ray_dir, hit_record);
	
	vec3 light_source = (vec3(10.*cos(iGlobalTime),10.*sin(iGlobalTime),10.)) ;
	vec3 light_color = vec3(.75,1.,.50);
	
	if (dst<MAX_DEPTH){
		vec3 pos = dst*ray_dir+ray_orign;
		vec3 nor = calcNormal(pos);
		
		vec3 lig = normalize(light_source - pos);
		
		vec3 ambient  = hit_record * .75;
		vec3 diffuse  = light_color * hit_record * clamp(dot(nor, lig), 0.0, 1.0);
		vec3 specular = light_color * hit_record * clamp(pow(dot(nor, lig),200.),0.0,1.0);
		
		color = ambient+diffuse+specular;
	}
	
	gl_FragColor = vec4(color,1.0);
}