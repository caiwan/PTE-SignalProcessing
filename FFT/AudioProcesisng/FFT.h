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
	double  re, im;
} complex;

inline void complex_exp(double r, double phi, struct complex_t *out){
	out->re = r * cos(phi);
	out->im = r * sin(phi);
}

inline void complex_mul(const complex *_a, const complex *_b, complex *out){
	double a=_a->re, b=_a->im, c=_b->re, d=_b->im;

	out->re = (a*c-b*d);
	out->im = (a*d+b*c);
}

inline void complex_mul_by_c(const complex *_a, const double _b, complex *out){
	double a=_a->re, b=_a->im;

	out->re = _b*a;
	out->im = _b*b;
}

inline void complex_add(const complex *_a, const complex *_b, complex *out){
	double a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a+c;
	out->im = b+d;
}

inline void complex_sub(const  complex *_a, const complex *_b, complex *out){
	double a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a-c;
	out->im = b-d;
}

// --------------------------------------------------------------

typedef struct int_complex_t{
	int re, im;
} intComplex;


inline void icomplex_mul(const intComplex *_a, const intComplex *_b, intComplex *out){
	int a=_a->re, b=_a->im, c=_b->re, d=_b->im;

	out->re = (a*c-b*d);
	out->im = (a*d+b*c);
}

inline void icomplex_mul_by_c(const intComplex *_a, const double _b, intComplex *out){
	int a=_a->re, b=_a->im;

	out->re = _b*a;
	out->im = _b*b;
}

inline void icomplex_add(const intComplex *_a, const intComplex *_b, intComplex *out){
	int a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a+c;
	out->im = b+d;
}

inline void icomplex_sub(const  intComplex *_a, const intComplex *_b, intComplex *out){
	int a=_a->re, b=_a->im, c=_b->re, d=_b->im;
	out->re = a-c;
	out->im = b-d;
}

//////////////////////////////////////////////////////////////////////////////
// 32-bit double Fast FT
/**
*/
class FFT {
	private:
		complex *Wn, *x, *y;	// y:= F{x}
		int samplesize;

		// CFFTv2 1:1-ben bekerul ide.
		inline void fft_ccopy ( int n, complex *x, complex *y ){  int i; for (i=0; i<n; i++ ) y[i] = x[i]; }
		void step (int n, int mj, complex *a, complex *b, complex *c, complex *d, double sgn);

	public:
		FFT(int samplesize, int samplerate);
		virtual ~FFT();

		void calculate(double *inbuf);
		void calculate();

		inline complex *getInputBuffer(){return this->x;}
		inline const complex *getLastResult(){return this->y;}
};

/// 32-bit Integer Fast-Fourier Transzformacio
/**
*/
class FFT32{
	private:
		intComplex *Wn, *x, *y;	// y:= F{x}
		int samplesize;

		// CFFTv2 1:1-ben bekerul ide.
		inline void fft_ccopy ( int n, intComplex *x, intComplex *y ){  int i; for (i=0; i<n; i++ ) y[i] = x[i]; }
		void step (int n, int mj, intComplex *a, intComplex *b, intComplex *c, intComplex *d, int sgn);

	public:
		FFT32(int samplesize, int samplerate);
		virtual ~FFT32();

		void calculate(int *inbuf);
		void calculate();

		inline intComplex *getInputBuffer(){return this->x;}
		inline const intComplex *getLastResult(){return this->y;}
};

#endif /*FFT_H*/

