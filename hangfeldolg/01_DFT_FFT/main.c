#include <stdio.h>
#include <stdlib.h>
//#define USE_MATH_DEFINES
#include <math.h>

#ifdef M_PI
#undef M_PI
#endif /*M_PI*/
//#define M_PI 3.14159265358979323846
#define M_PI 3.141592653589793238462643383279502884197169399375105820974944
// --------------------------------------------------------------
// muveletek komplex szamokkal

// komplex szamok reprezentalasahoz struktura
typedef float complex_elem_t;

typedef struct complex_t {
	complex_elem_t  re, im;
} complex;

// exponencialis alakbol felir egy komplex szamot
// r*e^j*phi
void complex_exp(float r, float phi, struct complex_t *out){
	out->re = r * cos(phi);
	out->im = r * sin(phi);
}

// ket komplex szam szorzata
void complex_mul(const complex *_a, const complex *_b, complex *out){
	float a=_a->re, b=_a->im, c=_b->re, d=_b->im;

	out->re = (a*c-b*d);
	out->im = (a*d+b*c);
}

// athelyezzuk lokalis valtozokba arra az esetre, ha
// a kimenet valamelyik bemenettel egyezne
void complex_mul_by_c(const complex *_a, const complex_elem_t _b, complex *out){
	float a=_a->re, b=_a->im;

	out->re = _b*a;
	out->im = _b*b;
}

// ket komplex szam  osszege
void complex_add(const complex *_a, const complex *_b, complex *out){
	float a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a+c;
	out->im = b+d;
}

// ket komplex szam  kulonbsege
void complex_sub(const  complex *_a, const complex *_b, complex *out){
	float a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a-c;
	out->im = b-d;
}

// --------------------------------------------------------------
// Discrete Foutirer Transform
void DFT(const complex *in, const unsigned int len, complex *out){
	int n=0, k=0;
	struct complex_t dk, op;
	float theta = 2*M_PI/(float)len, phi = 0.0;

	if (!in) return;
	if (!out) return;

	for(k=0; k<len; k++){
		dk.re = dk.im = 0.0;
		for(n=0; n<len; n++){
			op.im = op.re = 0.0;
			phi = -theta*(float)k*(float)n;		//printf("%3.2f; ", phi);
			complex_exp(1., phi, &op);
			complex_mul(&in[n], &op, &op);
			complex_add(&op, &dk, &dk);
		}
		//complex_mul_by_c(&dk, 1/(float)(len+1), &dk);
		out[k] = dk;
	}
}

// --------------------------------------------------------------
// Fast Foutirer Transform
// csak 2^n darab mintaval mukodik
// http://www.katjaas.nl/FFTimplement/FFTimplement.html

// ezt kivagtam innen, mert nem kell. 

// --------------------------------------------------------------
// cfft
// http://people.sc.fsu.edu/~jburkardt/c_src/fft_serial/fft_serial.c
/*
  Author:

    Original C version by Wesley Petersen.
    This C version by John Burkardt.
*/

// --------------------------------------------------------------
// modositott 

void fft_ccopy ( int n, complex *x, complex *y ){  int i; for (i=0; i<n; i++ ) y[i] = x[i]; }

// cfft init
void fft_cffti ( int n, complex *w){
  complex_elem_t arg;
  complex_elem_t aw;
  int i, n2;

  n2 = n / 2;
  aw = 2.0 * M_PI / ( ( float ) n );

  for ( i = 0; i < n2; i++ ){
    arg = aw * ((float)i);
	complex_exp(1., arg, &w[i]);
  }
}

void fft_step ( int n, int mj, complex *a, complex *b, complex *c, complex *d, complex *w, complex_elem_t sgn){
	complex amb;
	complex wjw;

	int j, ja, jb, jc, jd, jw, k, lj, mj2;
	mj2 = 2 * mj;
	lj  = n / mj2;

	for ( j = 0; j < lj; j++ ){
		jw = j * mj; ja  = jw; jb  = ja; jc  = j * mj2; jd  = jc;
		wjw = w[jw];
		if ( sgn < 0.0 ) wjw.im = -wjw.im;
		for ( k = 0; k < mj; k++ ){
			complex_add(&a[ja+k], &b[jb+k], &c[jc+k]);
			complex_sub(&a[ja+k], &b[jb+k], &amb);
			complex_mul(&wjw, &amb, &d[jd+k]);
    }
  }
  return;
}

// cfftv2
void fft_cfft2 ( int n, complex *x, complex *y, complex *w, complex_elem_t sgn ){
	int j, m, mj;
	int tgle;

	m = (int) (log((float)n)/log(1.99));
	mj   = 1;

	tgle = 1;
	fft_step( n, mj, &x[0], &x[(n/2)], &y[0], &y[mj], w, sgn );

	if ( n == 2 ) return;

	for ( j = 0; j < m - 2; j++ ){
		mj = mj * 2;
		if ( tgle ) {
			fft_step( n, mj, &y[0], &y[n/2], &x[0], &x[mj], w, sgn );
			tgle = 0;
    } else {
			fft_step( n, mj, &x[0], &x[n/2], &y[0], &y[mj], w, sgn );
      tgle = 1;
    }
  }

  if ( tgle ) {
    fft_ccopy ( n, y, x );
  }

  mj = n / 2;
  fft_step(n, mj, &x[0], &x[n/2], &y[0], &y[mj], w, sgn);

  return;
}

// --------------------------------------------------------------
// main

//float data[] = {0.f,1.f,2.f,3.f};

#define SIZEOF_ARRAY(x) (sizeof(x)/sizeof(*x))

int sampleTimes [] = {1,2,10};
int samples[] = {16, 32, 64, 128, 256};

int main(){
	int datasize, datasizeMax, i = 0, n = 0, t = 0;
	complex *src_dts, *Wn, *res_dft, *res_fft;
	float theta, omega, samplingTime, aw, af;

	//datasize = SIZEOF_ARRAY(data);
	datasize = datasizeMax = 1<<8;	//2^4 

	src_dts = (complex*)malloc(datasizeMax*sizeof(*src_dts));

	Wn = (complex*)malloc((datasizeMax/2)*sizeof(*Wn));

	//res_dft = (complex*)malloc(datasizeMax*sizeof(*res_dft));
	res_fft = (complex*)malloc(datasizeMax*sizeof(*res_fft));
	
	/////////////////////////////////////////////////////////
	// 1.

	datasize = 1<<6; // 2^6

	omega = M_PI*.1f;
	fft_cffti(datasize, Wn);

	for (i=0; i<SIZEOF_ARRAY(sampleTimes); i++){
		theta=omega*(float)sampleTimes[i];

		for (n=0; n<datasize; n++){
			src_dts[n].re = cos(theta*(float)n);
			src_dts[n].im = 0.f;
		}
	
		fft_cfft2(datasize, src_dts, res_fft, Wn, -1.0f);

		printf("N,f,omega,Re{D[N]},Im{D[N]},theta=%3.2f\n", theta);
		for(n=0; n<datasize; n++){
			//printf("D[%d] = %3.2f %c j*%3.2f \n", n, res_fft[n].re, (res_fft[n].im<0)?'-':'+', (res_fft[n].im<0)?-res_fft[n].im:res_fft[n].im);
			af = .5* (float)(1+n) / (float)sampleTimes[i];
			aw = 2.*M_PI * af;
			printf("%d,%3.2f,%3.2f,%3.2f,%3.2f\n", n, af, aw, res_fft[n].re, res_fft[n].im);
		}
	}

	/////////////////////////////////////////////////////////
	// 2.

	theta = omega;

	for (i=0; i<SIZEOF_ARRAY(samples); i++){

		datasize = samples[i];

		for (n=0; n<datasize; n++){
			src_dts[n].re = cos(theta*(float)n);
			src_dts[n].im = 0.f;
		}
	
		fft_cffti(datasize, Wn); 
		fft_cfft2(datasize, src_dts, res_fft, Wn, -1.0f);

		printf("N,omega,Re{D[N]},Im{D[N]},samples=%d\n", datasize);
		for(n=0; n<datasize; n++){
			//printf("D[%d] = %3.2f %c j*%3.2f \n", n, res_fft[n].re, (res_fft[n].im<0)?'-':'+', (res_fft[n].im<0)?-res_fft[n].im:res_fft[n].im);
			af = .5* (float)(1+n) / (float)sampleTimes[i];
			aw = 2.*M_PI * af;
			printf("%d,%3.2f,%3.2f,%3.2f,%3.2f\n", n, af, aw, res_fft[n].re, res_fft[n].im);
		}
	}

	free(Wn);
	free(src_dts);
	//free(res_dft); 
	free(res_fft); 
	return 0;
}
