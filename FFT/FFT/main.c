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

// ezzel megkonynitjuk a dolgunkat kicsit
#define complex_elem_t float

// komplex szamok reprezentalasahoz struktura
struct complex_t {
	complex_elem_t  re, im;
};

// exponencialis alakbol felir egy komplex szamot
// r*e^j*phi
void complex_exp(float r, float phi, struct complex_t *out){
	out->re = r * cos(phi);
	out->im = r * sin(phi);
}

// ket komplex szam szorzata
void complex_mul(struct complex_t *_a, struct complex_t *_b, struct complex_t *out){
	// athelyezzuk lokalis valtozokba arra az esetre, ha
	// a kimenet valamelyik bemenettel egyezne
	float a=_a->re, b=_a->im, c=_b->re, d=_b->im;

	out->re = (a*c-b*d);
	out->im = (a*d+b*c);
}

void complex_mul_by_c(struct complex_t *_a, float _b, struct complex_t *out){
	// athelyezzuk lokalis valtozokba arra az esetre, ha
	// a kimenet valamelyik bemenettel egyezne
	float a=_a->re, b=_a->im;

	out->re = _b*a;
	out->im = _b*b;
}

// ket komplex szam  osszege
void complex_add(struct complex_t *_a, struct complex_t *_b, struct complex_t *out){
	float a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a+c;
	out->im = b+d;
}

// ket komplex szam  kulonbsege
void complex_sub(struct complex_t *_a, struct complex_t *_b, struct complex_t *out){
	struct complex_t tmp = {-_b->re, -_b->im};
	complex_add(_a, &tmp, out);
}

// --------------------------------------------------------------
// Discrete Foutirer Transform
void DFT(const float *in, const unsigned int len, struct complex_t *out){
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
			complex_exp(in[n], phi, &op);
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
void FFT(unsigned int logN, const complex_elem_t* in, struct complex_t* out)
{
	int i=0, j=0;
	unsigned int N = 1<<logN;
	struct complex_t *out_n, *out_nspan;

	for(i=0; i<N; i++) {out[i].re = in[i]; out[i].im = 0;}

	for(j=0; j<(N>>1); j++){
		for(i=0; i<logN; i++){
			out[2*j+0].re += out[2*j+0].re;
			out[2*j+0].im += out[2*j+0].im;

			out[2*j+1].re += out[2*j+1].re;
			out[2*j+1].im += out[2*j+1].im;
		}
	}
}

void FFT_recursive(unsigned int logN, const complex_elem_t* in, struct complex_t* out)
{
	int i=0, j=0;
	unsigned int N = 1<<logN;
	struct complex_t *out_n, *out_nspan;

	for(i=0; i<N; i++) {out[i].re = in[i]; out[i].im = 0;}

	for(j=0; j<(N>>1); j++){
		for(i=0; i<logN; i++){
			out[2*j+0].re += out[2*j+0].re;
			out[2*j+0].im += out[2*j+0].im;

			out[2*j+1].re += out[2*j+1].re;
			out[2*j+1].im += out[2*j+1].im;
		}
	}
}

void FFT2(unsigned int logN, const complex_elem_t* in, struct complex_t* out) // logN is base 2 log(N)
{
	unsigned int i = 0, n=0, nspan, span, submatrix, node;
	unsigned int N = 1<<logN;
	double temp, primitive_root, angle, realtwiddle, imtwiddle;

	struct complex_t *out_n, *out_nspan;

	for(i=0; i<N; i++) {out[i].re = in[i]; out[i].im = 0;}

	for(span=N>>1; span; span>>=1){

		primitive_root = 2*M_PI/span;
		for(submatrix=0; submatrix<(N>>1)/span; submatrix++){
			for(node=0; node<span; node++){
				nspan = n+span;

				out_nspan = &out[nspan];
				out_n = &out[n];

				temp = out_n->re + out_nspan->re;

				out_nspan->re = out_n->re - out_nspan->re;
				out_n->re = temp;

				temp = out_n->im + out_nspan->im;
				out_nspan->im = out_n->im - out_nspan->im;
				out_n->im = temp;

				angle = primitive_root * node;      // rotations
				realtwiddle = cos(angle);
				imtwiddle = sin(angle); //printf("%3.2f ", angle);

				temp = realtwiddle * out_nspan->re - imtwiddle * out_nspan->im;
				out_nspan->im = realtwiddle * out_nspan->im + imtwiddle * out_nspan->re;
				out_nspan->re = temp;				

				n++;
            
          }
         
          n = (n+span) & (N-1);
        
        } 
     } 
}
// --------------------------------------------------------------
// cfft
// http://people.sc.fsu.edu/~jburkardt/c_src/fft_serial/fft_serial.c
/*
  Author:

    Original C version by Wesley Petersen.
    This C version by John Burkardt.
*/

// complex copy
void ccopy ( int n, complex_elem_t x[], complex_elem_t y[] ){ int i; for ( i = 0; i < n; i++ ) { y[i*2+0] = x[i*2+0]; y[i*2+1] = x[i*2+1]; } return; }

// cfft init
void cffti ( int n, complex_elem_t w[] ){
  complex_elem_t arg;
  complex_elem_t aw;
  int i;
  int n2;

  n2 = n / 2;
  aw = 2.0 * M_PI / ( ( double ) n );

  for ( i = 0; i < n2; i++ )
  {
    arg = aw * ( ( double ) i );
    w[i*2+0] = cos ( arg );
    w[i*2+1] = sin ( arg );
  }
  return;
}

void step ( int n, int mj, complex_elem_t a[], complex_elem_t b[], complex_elem_t c[], complex_elem_t d[], complex_elem_t w[], complex_elem_t sgn ){
  complex_elem_t ambr;
  complex_elem_t ambu;
  int j, ja, jb, jc, jd, jw;
  int k, lj, mj2;
  complex_elem_t wjw[2];

  mj2 = 2 * mj;
  lj  = n / mj2;

  for ( j = 0; j < lj; j++ )
  {
    jw = j * mj;
    ja  = jw;
    jb  = ja;
    jc  = j * mj2;
    jd  = jc;

    wjw[0] = w[jw*2+0]; 
    wjw[1] = w[jw*2+1];

    if ( sgn < 0.0 ) 
    {
      wjw[1] = - wjw[1];
    }

    for ( k = 0; k < mj; k++ )
    {
      c[(jc+k)*2+0] = a[(ja+k)*2+0] + b[(jb+k)*2+0];
      c[(jc+k)*2+1] = a[(ja+k)*2+1] + b[(jb+k)*2+1];

      ambr = a[(ja+k)*2+0] - b[(jb+k)*2+0];
      ambu = a[(ja+k)*2+1] - b[(jb+k)*2+1];

      d[(jd+k)*2+0] = wjw[0] * ambr - wjw[1] * ambu;
      d[(jd+k)*2+1] = wjw[1] * ambr + wjw[0] * ambu;
    }
  }
  return;
}

// cfftv2
void cfft2 ( int n, complex_elem_t x[], complex_elem_t y[], complex_elem_t w[], complex_elem_t sgn ){
  int j, m, mj, tgle;

   m = ( int ) ( log ( ( double ) n ) / log ( 1.99 ) );
   mj   = 1;
/*
  Toggling switch for work array.
*/
  tgle = 1;
  step ( n, mj, &x[0*2+0], &x[(n/2)*2+0], &y[0*2+0], &y[mj*2+0], w, sgn );

  if ( n == 2 ){
    return;
  }

  for ( j = 0; j < m - 2; j++ ){
    mj = mj * 2;
    if ( tgle )
    {
      step ( n, mj, &y[0*2+0], &y[(n/2)*2+0], &x[0*2+0], &x[mj*2+0], w, sgn );
      tgle = 0;
    }
    else
    {
      step ( n, mj, &x[0*2+0], &x[(n/2)*2+0], &y[0*2+0], &y[mj*2+0], w, sgn );
      tgle = 1;
    }
  }
/* 
  Last pass through data: move Y to X if needed.
*/
  if ( tgle ) {
    ccopy ( n, y, x );
  }

  mj = n / 2;
  step ( n, mj, &x[0*2+0], &x[(n/2)*2+0], &y[0*2+0], &y[mj*2+0], w, sgn );

  return;
}

// --------------------------------------------------------------
// main

float data[] = {0.f,1.f,2.f,3.f};

int main(){
	int datasize = sizeof(data)/sizeof(*data), i = 0;
	
	struct complex_t *src_dts = (struct complex_t*)malloc(datasize*sizeof(*src_dts));

	complex_elem_t *Wn = (complex_elem_t*)malloc(datasize*sizeof(*Wn));

	struct complex_t *res_dft = (struct complex_t*)malloc(datasize*sizeof(*res_dft));
	struct complex_t *res_fft = (struct complex_t*)malloc(datasize*sizeof(*res_fft));

	for (i=0; i<datasize; i++){src_dts[i].re = data[i]; src_dts[i].im = 0.f;}

	cffti(datasize, Wn); 

	DFT(data, datasize, res_dft);
	//FFT(2, data, res_fft);

	cfft2(datasize, (complex_elem_t*)src_dts, (complex_elem_t*)res_dft, Wn, +1.0f);
	//cfft2(datasize, (complex_elem_t*)src_dts, (complex_elem_t*)res_dft, Wn, -1.0f);

	printf("DFT:\n");
	for(i=0; i<datasize; i++){
		printf("D[%d] = %3.2f %c j*%3.2f \n", i, res_dft[i].re, (res_dft[i].im<0)?'-':'+', (res_dft[i].im<0)?-res_dft[i].im:res_dft[i].im);
	}

	printf("\nRadix-2 FFT:\n");
	for(i=0; i<datasize; i++){
		printf("D[%d] = %3.2f %c j*%3.2f \n", i, res_fft[i].re, (res_fft[i].im<0)?'-':'+', (res_fft[i].im<0)?-res_fft[i].im:res_fft[i].im);
	}

	free(res_dft); 
	free(res_fft); 
	return 0;
}
