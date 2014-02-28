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
			phi = -theta*(float)k*(float)n;
			complex_exp(in[n], phi, &op);
			complex_add(&op, &dk, &dk);
		}
		complex_mul_by_c(&dk, 1/(float)(len+1), &dk);
		out[k] = dk;
	}
}

// --------------------------------------------------------------
// Fast Foutirer Transform
// csak 2^n darab mintaval mukodik
// http://www.katjaas.nl/FFTimplement/FFTimplement.html
void FFT(unsigned int logN, double *real, double *im) // logN is base 2 log(N)
{
    unsigned int n=0, nspan, span, submatrix, node;
    unsigned int N = 1<<logN;
    double temp, primitive_root, angle, realtwiddle, imtwiddle;

      

    for(span=N>>1; span; span>>=1)      // loop over the FFT stages
    {
       primitive_root = MINPI/span;     // define MINPI in the header
       
       for(submatrix=0; submatrix<(N>>1)/span; submatrix++)
       {
          for(node=0; node<span; node++)
          {
            nspan = n+span;
            temp = real[n] + real[nspan];       // additions & subtractions
            real[nspan] = real[n]-real[nspan];
            real[n] = temp;
            temp = im[n] + im[nspan];
            im[nspan] = im[n] - im[nspan];
            im[n] = temp;
            
            angle = primitive_root * node;      // rotations
            realtwiddle = cos(angle);
            imtwiddle = sin(angle);
            temp = realtwiddle * real[nspan] - imtwiddle * im[nspan];
            im[nspan] = realtwiddle * im[nspan] + imtwiddle * real[nspan];
            real[nspan] = temp;
            
            n++;   // not forget to increment n
            
          } // end of loop over nodes
         
          n = (n+span) & (N-1);   // jump over the odd blocks
        
        } // end of loop over submatrices
        
     } // end of loop over FFT stages
} // end of FFT function

// --------------------------------------------------------------
// main

float data[] = {0.f,1.f,2.f,3.f};

int main(){
	int datasize = sizeof(data)/sizeof(*data),
		i = 0;
	struct complex_t *res_dft = malloc(datasize*sizeof(*res_dft));

	DFT(data, datasize, res_dft);

	for(i=0; i<datasize; i++){
		printf("D[%d] = %3.2f %c j*%3.2f \n", i, res_dft[i].re, (res_dft[i].im<0)?'-':'+', (res_dft[i].im<0)?-res_dft[i].im:res_dft[i].im);
	}

	free(res_dft); 
	return 0;
}
