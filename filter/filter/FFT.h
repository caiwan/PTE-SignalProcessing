#ifndef FFT_H
#define FFT_H

#include <math.h>

#ifdef M_PI
#undef M_PI
#endif /*M_PI*/
//#define M_PI 3.14159265358979323846
#define M_PI 3.141592653589793238462643383279502884197169399375105820974944

// --------------------------------------------------------------
// muveletek komplex szamokkal

typedef struct complex_t {
	float  re, im;
} complex;

inline void complex_exp(float r, float phi, struct complex_t *out){
	out->re = r * cos(phi);
	out->im = r * sin(phi);
}

inline void complex_mul(const complex *_a, const complex *_b, complex *out){
	float a=_a->re, b=_a->im, c=_b->re, d=_b->im;

	out->re = (a*c-b*d);
	out->im = (a*d+b*c);
}

inline void complex_mul_by_c(const complex *_a, const float _b, complex *out){
	float a=_a->re, b=_a->im;

	out->re = _b*a;
	out->im = _b*b;
}

inline void complex_add(const complex *_a, const complex *_b, complex *out){
	float a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a+c;
	out->im = b+d;
}

inline void complex_sub(const  complex *_a, const complex *_b, complex *out){
	float a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a-c;
	out->im = b-d;
}

//////////////////////////////////////////////////////////////////////////////
class FFT {
	private:
		complex *Wn, *x, *y;	// y:= F{x}
		int samplesize;

		// CFFTv2 1:1-ben bekerul ide.
		inline void fft_ccopy ( int n, complex *x, complex *y ){  int i; for (i=0; i<n; i++ ) y[i] = x[i]; }
		void step (int n, int mj, complex *a, complex *b, complex *c, complex *d, float sgn);

	public:
		FFT(int samplesize, int samplerate);
		virtual ~FFT();

		void calculate(float *inbuf);
		void calculate();

		inline const complex_t *getInputBuffer(){return this->x;}
		inline const complex_t *getLastResult(){return this->y;}
};

#endif /*FFT_H*/

