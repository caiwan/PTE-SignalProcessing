#ifndef FFT_H
#define FFT_H

#include <math.h>
// --------------------------------------------------------------
// muveletek komplex szamokkal

typedef float complex_elem_t;
typedef struct complex_t {
	complex_elem_t  re, im;
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

inline void complex_mul_by_c(const complex *_a, const complex_elem_t _b, complex *out){
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
		complex *Wn, *x, *y;
		int samplesize;

		// CFFTv2 1:1-ben bekerul ide.

	public:
		FFT(int samplesize);
		virtual ~FFT();

		void calculate();

		inline const complex_t *getLastResult(){return this->y;}
};

#endif /*FFT_H*/

