#include "FFT.h"


FFT::FFT(int samplesize, int samplerate){
	this->Wn = new complex[samplesize / 2];
	this->x = new complex[samplesize];
	this->y = new complex[samplesize];

	this->samplesize = samplesize;

	// init W(n)
	float arg, aw;
	int i, n2;

	n2 = samplesize / 2;
	//aw = 2.0 * M_PI / ((float)samplesize );
	aw = 2.0 * M_PI / ((float)samplerate);

	for ( i = 0; i < n2; i++ ){
		arg = aw * ((double)i);
		complex_exp(1., arg, &this->Wn[i]);
	}
}

FFT::~FFT(void){
	if (this->Wn) delete this->Wn;
	if(this->x) delete this->x;
	if(this->y) delete this->y;
}

void FFT::calculate(){
	float sgn = -1.;
	int j, m, mj, n = this->samplesize;
	int tgle;

	m = (int) (log ((float)n) / log ( 1.99 ) );
	mj   = 1;

	tgle = 1;
	this->step( n, mj, &x[0], &x[(n/2)], &y[0], &y[mj], sgn );

	if ( n == 2 ) return;

	for ( j = 0; j < m - 2; j++ ){
		mj = mj * 2;
		if ( tgle ) {
			this->step( n, mj, &y[0], &y[n/2], &x[0], &x[mj], sgn );
			tgle = 0;
    } else {
		this->step( n, mj, &x[0], &x[n/2], &y[0], &y[mj], sgn );
		tgle = 1;
    }
  }

  if ( tgle ) {
    fft_ccopy ( n, y, x );
  }

  mj = n / 2;
  this->step(n, mj, &x[0], &x[n/2], &y[0], &y[mj], sgn);

  return;
}

void FFT::step(int n, int mj, complex *a, complex *b, complex *c, complex *d, float sgn){
	complex amb;
	complex wjw;

	int j, ja, jb, jc, jd, jw, k, lj, mj2;
	mj2 = 2 * mj;
	lj  = n / mj2;

	for ( j = 0; j < lj; j++ ){
		jw = j * mj; ja  = jw; jb  = ja; jc  = j * mj2; jd  = jc;
		wjw = this->Wn[jw];
		if ( sgn < 0.0 ) wjw.im = -wjw.im;
		for ( k = 0; k < mj; k++ ){
			complex_add(&a[ja+k], &b[jb+k], &c[jc+k]);
			complex_sub(&a[ja+k], &b[jb+k], &amb);
			complex_mul(&wjw, &amb, &d[jd+k]);
    }
  }
  return;
}

