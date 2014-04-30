#ifdef GL_ES
precision mediump float;
#endif

#define MAX_DEPTH 60.
#define MAX_ITERATION 64
#define MIN_PRECISSION 0.0001

//----------------------------------------------------------------------
float sdPlane( vec3 p )
{
	return p.y+0.2;
}

float sdSphere( vec3 p, float s )
{
    return length(p)-s;
}

float udRoundBox( vec3 p, vec3 b, float r )
{
  return length(max(abs(p)-b,0.0))-r;
}

float length2( vec2 p )
{
	return sqrt( p.x*p.x + p.y*p.y );
}

float length6( vec2 p )
{
	p = p*p*p; p = p*p;
	return pow( p.x + p.y, 1.0/6.0 );
}

float length8( vec2 p )
{
	p = p*p; p = p*p; p = p*p;
	return pow( p.x + p.y, 1.0/8.0 );
}

//----------------------------------------------------------------------

float opS( float d1, float d2 )
{
    return max(-d2,d1);
}

float opU( float d1, float d2 )
{
	return (d1<d2) ? d1 : d2;
}


vec2 opU( vec2 d1, vec2 d2 )
{
	return (d1.x<d2.x) ? d1 : d2;
}

vec3 opRep( vec3 p, vec3 c )
{
    return mod(p,c)-0.5*c;
}

vec3 opTwist( vec3 p )
{
    float  c = cos(10.0*p.y+10.0);
    float  s = sin(10.0*p.y+10.0);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}

vec3 opTwist( vec3 p, float tc, float ts )
{
    float  c = cos(tc*p.y+ts);
    float  s = sin(tc*p.y+ts);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}

vec3 opTwist2( vec3 p, float tc, float ts, float cc)
{
    float  c = cos(cc*tc*p.y+ts);
    float  s = sin(cc*tc*p.y+ts);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}

vec3 opTwist3( vec3 p, float tc, float ts, float cc)
{
    float  c = cos(sin(tc*p.y)*cc*tc*p.y+ts);
    float  s = sin(sin(tc*p.y)*cc*tc*p.y+ts);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}

//----------------------------------------------------------------------

float map(in vec3 pos, out vec3 color)
{
	float r = udRoundBox(opTwist3(pos, 5.0, iGlobalTime, sin(iGlobalTime)), vec3(0.2,0.2,1.0), 0.1);	
	color = vec3(.2); 
	return r;
}

//----------------------------------------------------------------------

float intersect(in vec3 ro, in vec3 rd, out vec3 color)
{
    float h = MIN_PRECISSION*2.0;
    float t = 0.0;
    
	color = vec3(0.);
		
    for( int i=0; i<MAX_ITERATION; i++ )
    {
        if( abs(h)<MIN_PRECISSION||t>MAX_DEPTH ) break;
        t += h;
	    h = map( ro+rd*t, color );
    }

    return t;
}


float softshadow( in vec3 ro, in vec3 rd, in float mint, in float maxt, in float k )
{
	float res = 1.0;
    float dt = 0.02;
    float t = mint;
	vec3 c;
    for( int i=0; i<20; i++ )
    {
		if( t<maxt )
		{
        float h = map( ro + rd*t, c );
        res = min( res, k*h/t );

	t += 0.02;
			
		}
    }
    return clamp( res, 0.0, 1.0 );

}

vec3 calcNormal( in vec3 pos )
{
	vec3 c;
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    map(pos+eps.xyy, c) - map(pos-eps.xyy, c),
	    map(pos+eps.yxy, c) - map(pos-eps.yxy, c),
	    map(pos+eps.yyx, c) - map(pos-eps.yyx, c));
	return normalize(nor);
}

float calcAO( in vec3 pos, in vec3 nor )
{
	float totao = 0.0;
    float sca = 1.0;
	vec3 c;
    for( int aoi=0; aoi<5; aoi++ )
    {
        float hr = 0.01 + 0.05*float(aoi);
        vec3 aopos =  nor * hr + pos;
        float dd = map(aopos, c);
        totao += -(dd-hr)*sca;
        sca *= 0.75;
    }
    return clamp( 1.0 - 4.0*totao, 0.0, 1.0 );
}

//----------------------------------------------------------------------

void main( void )
{
    vec2 pos = -1.0+2.0*(gl_FragCoord.xy/iResolution.xy);
	pos.x *= iResolution.x / iResolution.y;
		 
	// camera	
	vec3 eye = vec3( 0.0, 0.5 , 1.5);					// szemkoordinata (ray orign)
	vec3 center = vec3( 0.0, 0.0, 0.0 );					// referenciapont
	
	// camera tx
	vec3 cw = normalize(center-eye);						// szem -> ref.pont = nezet irany
	vec3 cp = vec3( 0.0, 1.0, 0.0 );						// "felfele" vektor
	vec3 cu = normalize( cross(cw,cp) );					// nezet irany X felfele mutato vektor = up iranyvektor
	vec3 cv = normalize( cross(cu,cw) );					// up X nezet irany
	vec3 rayDir = normalize( pos.x*cu + pos.y*cv + 2.5*cw );	// sugar irany (ray dir.)

	// render forras->irany
	//vec3 col = render(eye, rayDir);							// render forras->irany
	//////////////
	vec3 col = vec3(0.0);
    float d = intersect(eye, rayDir, col);
    if(d>0. && d<MAX_DEPTH);
    {
        vec3 pos = eye + d*rayDir;
        vec3 nor = calcNormal( pos );

		vec3 col2 = vec3(.4, .4, .02);
		
        float ao = calcAO( pos, nor );

		vec3 lig = normalize( vec3(-1.0, 1.0, 1.0) );	// feny iranya
		float amb = clamp( 0.5+0.5*nor.y, 0.0, 1.0 );
        float dif = clamp( dot( nor, lig ), 0.0, 1.0 );
        float bac = clamp( dot( nor, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-pos.y,0.0,1.0);

		float sh = 1.0;
		if( dif>0.02 ) { sh = softshadow( pos, lig, 0.02, 10.0, 7.0 ); dif *= sh; }

		vec3 brdf = vec3(0.0);
		
		brdf += 0.20*amb*vec3(0.20,0.00,0.00)*ao;	// ambient
		brdf += 0.20*bac*vec3(0.15,0.0,0.15)*ao;		// back light
		brdf += 1.20*dif*vec3(1.00,0.90,0.60);		// diffusal
		
		float pp = clamp( dot( reflect(rayDir,nor), lig ), 0.0, 1.0 );
		float spe = sh*pow(pp,16.0);	// specular
		float fre = ao*pow( clamp(1.0+dot(nor,rayDir),0.0,1.0), 2.0 );

		//col = col*brdf + vec3(1.0)*col*spe + 0.2*fre*(0.5+0.5*col);
		col = col*brdf + vec3(1.0)*col*spe + mix(col,col2, 0.4*fre);
	}

	col *= exp( -0.01*d*d);
	col *= col;
	
	col = clamp(col,0.0,1.0);
	//////////////

	col = sqrt( col );

    gl_FragColor=vec4( col, 1.0 );
}
