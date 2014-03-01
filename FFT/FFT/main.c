#include <stdio.h>
#include <stdlib.h>
#define USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif /*M_PI*/

// --------------------------------------------------------------
// muveletek komplex szamokkal

// komplex szamok reprezentalasahoz struktura
struct complex_t {
	float re, im;
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
void FFT(unsigned int logN, struct complex_t* in, struct complex_t* out)
{
	// ... 
}

void FFT2(unsigned int logN, const float* in, struct complex_t* out) // logN is base 2 log(N)
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
				
				//in_n.re = in[n];		in_n.im = 0.0f;
				//in_nspan.re = in[nspan];	in_nspan.im = 0.0f;

				out_nspan = &out[nspan];
				out_n = &out[n];

//				temp = real[n] + real[nspan];       // additions & subtractions
				temp = out_n->re + out_nspan->re;
//				real[nspan] = real[n]-real[nspan];
				out_nspan->re = out_n->re - out_nspan->re;
///				real[n] = temp;
				out_n->re = temp;

//				temp = im[n] + im[nspan];
				temp = out_n->im + out_nspan->im;
//				im[nspan] = im[n] - im[nspan];
				out_nspan->im = out_n->im - out_nspan->im;
//				im[n] = temp;
				out_n->im = temp;

				angle = primitive_root * node;      // rotations
				realtwiddle = cos(angle);
				imtwiddle = sin(angle); //printf("%3.2f ", angle);

//				temp = realtwiddle * real[nspan] - imtwiddle * im[nspan];
				temp = realtwiddle * out_nspan->re - imtwiddle * out_nspan->im;
//				im[nspan] = realtwiddle * im[nspan] + imtwiddle * real[nspan];
				out_nspan->im = realtwiddle * out_nspan->im + imtwiddle * out_nspan->re;
//				real[nspan] = temp;
				out_nspan->re = temp;				

				n++;
            
          }
         
          n = (n+span) & (N-1);
        
        } 
     } 
}

// --------------------------------------------------------------
// main

float data[] = {0.f,1.f,2.f,3.f,4.f,5.f,6.f,7.f};

int main(){
	int datasize = sizeof(data)/sizeof(*data),
		i = 0;
	struct complex_t *res_dft = (struct complex_t*)malloc(datasize*sizeof(*res_dft));
	struct complex_t *res_fft = (struct complex_t*)malloc(datasize*sizeof(*res_fft));

	DFT(data, datasize, res_dft);
	FFT2(, data, res_fft);

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
