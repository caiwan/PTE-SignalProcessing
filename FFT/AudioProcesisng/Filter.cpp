#include "Filter.h"

FreqFilter::Filter::Filter(){
	this->freq_chr = NULL;
}

FreqFilter::Filter::~Filter(){
	if (this->freq_chr) delete this->freq_chr;
}

void FreqFilter::Filter::init(int freqstep, int bufsze){
	if (this->freq_chr) delete this->freq_chr;
	this->freq_chr = new complex[bufsze];

	this->sampling = freqstep;

	float freq = (float) bufsze / (float) freqstep, fq = 0.0f;

	for(int i=0; i<bufsze; ++i){
		fq = freq*(1+(float)i);
		this->freq_chr[i] = this->getFreq(fq);
	}
}

///////////////////////////////////////////////////////////

FreqFilter::FilterChain::FilterChain(int _windowsize, int _samplerate){
	this->windowsize =_windowsize;
	this->samplerate =_samplerate;
}

void FreqFilter::FilterChain::calculate(const complex* inbuf, complex *outbuf){
	for(int i=0; i<this->filterList.size(); i++){
		const complex *q = this->filterList[i]->getFqChar();
		for (int k=0; k<this->windowsize; k++){
			if (!k) 
				outbuf[k].re = q[k].re, outbuf[k].im = q[k].im;
			else 
				complex_mul(&q[k], &outbuf[k], &outbuf[k]);
		}

		for (int k=0; k<this->windowsize; k++){
			complex_mul(&inbuf[k], &outbuf[k], &outbuf[k]);
		}
	}
}

FreqFilter::FilterChain::~FilterChain(){
	this->filterList.clear();
}

///////////////////////////////////////////////////////////

FreqFilter::FilterLowPass::FilterLowPass(float _cutFreq, float _falloff, float _phaseShift) : Filter(){
	this->cutFreq = _cutFreq;
	this->falloff = _falloff;
	this->phaseShift = _phaseShift;
}

FreqFilter::FilterLowPass::~FilterLowPass(){
}

complex FreqFilter::FilterLowPass::getFreq(float freq){
	complex out = {0.,0.};
	if (freq<this->cutFreq) return out;
	out.re = 1.0;
	return out;
}