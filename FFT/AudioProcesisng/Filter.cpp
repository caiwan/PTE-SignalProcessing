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

FreqFilter::FilterLowPass::FilterLowPass(float _cutFreq, float _falloff, float _phaseShift) : Filter(){
	this->cutFreq = _cutFreq;
	this->falloff = _falloff;
	this->phaseShift = _phaseShift;
}

complex FreqFilter::FilterLowPass::getFreq(float freq){
	complex out = {0.,0.};
	if (freq<this->cutFreq) return out;
	out.re = 1.0;
	return out;
}