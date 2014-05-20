#include "Filter.h"

#include <cmath>

#undef M_PI
#define M_PI       3.14159265358979323846

////
void BmxResonanceFilter(float *psamples, int numsamples, float f, float q, float *b0, float *b1);

///// 
// filter chain
Filter::Chain::Chain(int samplerate) : Generator(), filterChain(){
	this->samplerate = samplerate;
	this->sampletime = 1./(float)samplerate;

	this->buffer[0] = NULL;
	this->buffer[1] = NULL;
	this->generator = NULL;
}

void Filter::Chain::allocate(int len){
	if (!buffer[0] && len){
		//this->buffer[0] = malloc(len*sizeof(this->buffer[0]));
		this->buffer[0] = new float [len];
		this->buffer[1] = new float [len];
		//this->buffer[1] = malloc(len*sizeof(this->buffer[0]));
		this->bufsize = len;
	} else if (this->bufsize != len || len == 0){
		//free(this->buffer[0]);
		//free(this->buffer[1]);
		if (this->buffer[0]) delete this->buffer[0];
		if (this->buffer[1]) delete this->buffer[1];

		this->buffer[0] = NULL;
		this->buffer[1] = NULL;
		if (len) this->allocate(len);
	}

	this->inbuf = buffer[0];
	this->outbuf = buffer[0];
}

void Filter::Chain::render(int len, int offset, WavRead::channel_t channel ){
	if (!this->generator) throw 21;
	//if (this->filterChain.empty()) throw 22;

	this->allocate(len);
	this->generator->fillBufferFloat(this->inbuf, len, offset, channel);

	Filter *actfilter = NULL;

	for (int i=0; i<this->filterChain.size(); i++){
		actfilter = this->filterChain[i];
		
		if (actfilter) actfilter->render(this->inbuf, this->outbuf, this->bufsize);

		this->swapBuffers();
	}

	this->swapBuffers();
}

void Filter::Chain::fillBufferU16(unsigned short *data, int len, int offset, WavRead::channel_t channel){
	this->render(len, offset, channel);
	throw 28;
}

void Filter::Chain::fillBufferInt(int *data, int len, int offset, WavRead::channel_t channel){
	this->render(len, offset, channel);
	throw 28;
}

void Filter::Chain::fillBufferFloat(float *data, int len, int offset, WavRead::channel_t channel){
	this->render(len, offset, channel);
	memcpy(data, this->outbuf, len*sizeof(*data));
}

Filter::Chain::~Chain(){
	this->allocate(0);
	if (this->generator) delete this->generator;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
Filter::Generator::Generator(){}
Filter::Generator::~Generator(){}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// WAV READER
Filter::GeneratorWav::GeneratorWav(WavRead *_reader) : Generator() {
	if (!reader) throw 1;
	this->reader = _reader;
	this->reader->fillBuffer();

	this->buffer[0] = new float[2*AUDIO_BUFFER_LEN];
	this->buffer[1] = new float[2*AUDIO_BUFFER_LEN];
	this->bpos = 0;
	//this->reader->fillBufferComplex(this->buffer, WavRead::CH_MONO);
}

void Filter::GeneratorWav::fillBufferFloat(float *data, int len, int offset, WavRead::channel_t channel){
	if (len <= AUDIO_BUFFER_LEN){
		this->reader->fillBufferComplex(data, channel, len, 1);
	} else {
		throw 25;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// SAW
Filter::GanaratorSaw::GanaratorSaw() : Generator() {}
Filter::GanaratorSaw::GanaratorSaw(float freq, float sampling, float falloff) : Generator() {
	this->tick(freq,  sampling, falloff);
}

Filter::GanaratorSaw::~GanaratorSaw(){}

void Filter::GanaratorSaw::tick(float _freq, float _sampling, float _falloff){
	this->freq = _freq;
	this->sampling = _sampling;
	this->falloff = _falloff;
	this->s = this->u = 0.;
	this->ts = 1. / this->sampling;
	this->ff = this->freq / this->sampling;
	this->fu = this->falloff / this->sampling;
}

void Filter::GanaratorSaw::fillBufferFloat(float *data, int len, int offset, WavRead::channel_t channel){
	//float t0 = (float)offset / this->sampling;
	//float t;
	for(int i=0; i<len; i++){
		this->u += this->fu;
 		if (this->u>.999) this->u = 0.;

		this->s += this->ff;
		if (this->s>.999) this->s = 0.;
		
		data[i] = (1-this->s-.5)*(1.-this->u);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Filterek
// FFI Moving Avg

void Filter::Filter_FIR_MA:: render(float* inbuf, float* outbuf, int length){
	float out;

	for (int i = 0; i < length; i++)
	{
		out = 0.0;
		int pp = this->M>i?i:this->M;
		for (int p=0; p<pp; p++){
			out += 1./(float)(this->M) * inbuf[i-p];
		}

		outbuf[i]=out; 
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
// Filterek
// FFI Sinc

Filter::Filter_FIR_Sinc::Filter_FIR_Sinc(int _m, float _sigma, float _sampling){
	this->impulse = new float[_m];
	this->M = _m;
	this->simga = _sigma;
	float ts = 1./_sampling, t;

	for(int i=0; i<_m; i++){
		if(i){
			//t = ts*(float) i;
			t = (float) i-.5*(float)_m;
			this->impulse[i] = sin(2*M_PI*_sigma*t)/(M_PI*t);
		} else {
			this->impulse[i] = 1.;
		}
	}
}

Filter::Filter_FIR_Sinc::~Filter_FIR_Sinc(){
	if(this->impulse) delete this->impulse;
}

void Filter::Filter_FIR_Sinc::render(float* inbuf, float* outbuf, int length){
	float out;

	for (int i = 0; i < length; i++)
	{
		out = 0.0;
		int pp = this->M>i?i:this->M;
		for (int p=0; p<pp; p++){
			out += this->impulse[p] * inbuf[i-p];
		}

		outbuf[i]=out; 
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Filterek
// IIF Resonance

Filter::Filter_IIR_Resonance::Filter_IIR_Resonance() : Filter(){
	this->buf[0] = 0.;
	this->buf[1] = 0.;
	this->resonance = 0.1;
	this->cutoff = 0.1;

	this->sampling = 44100;
	this->res_lfo = 0.;
	this->cut_lfo = 0.;

	this->t0 = 0;
	this->ts = 1./this->sampling;
}

Filter::Filter_IIR_Resonance::Filter_IIR_Resonance(float _cut_lfo, float _res_lfo, float _sampling) : Filter(){
	this->buf[0] = 0.;
	this->buf[1] = 0.;
	this->resonance = 0.;
	this->cutoff = 0.;

	this->sampling = _sampling;
	this->res_lfo = _res_lfo / this->sampling;
	this->cut_lfo = _cut_lfo / this->sampling;

	this->t0 = 0;
	this->ts = 1./this->sampling;
}

Filter::Filter_IIR_Resonance::~Filter_IIR_Resonance(){
}

void Filter::Filter_IIR_Resonance::render(float *inbuf, float *outbuf, int len){
	double in, fb, out;
	double f = this->cutoff, q = this->resonance;

	float t = 0;

	fb = q + q / (1.0 - f);

	for (int i = 0; i < len; i++)
	{
		if (this->cut_lfo > 0.){
			t = this->t0 + (float) i * this->ts;
			
			this->tick(
				0.475 * (1. + cos(t*this->cut_lfo*M_PI)), 
				0.475 * (1. + sin(t*this->res_lfo*M_PI))
			);
			
			this->t0 = t;

			f = this->cutoff, q = this->resonance;
			fb = q + q / (1.0 - f);
		}

		in = inbuf[i];

		buf[0] = buf[0] + f * (in - buf[0] + fb * (buf[0] - buf[1]));
		buf[1] = buf[1] + f * (buf[0] - buf[1]);

		out = (float)buf[1];

		if (out>1.) out = 1.;
		if (out<-1.) out = -1.;

		outbuf[i] = out;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
/**
	@param psamples mintak
	@param numsamples mintak szama
	@param f cutoff [0.1 .. 0.99]
	@param q resonance [0.0 .. 0.88]
*/
void BmxResonanceFilter(float *psamples, int numsamples, float f, float q, float *b0, float *b1)
{
	float in, fb, buf0, buf1;

	buf0 = *b0;
	buf1 = *b1;

	fb = q + q / (1.0 - f);

	for (int i = 0; i < numsamples; i++)
	{
		in = psamples[i];

		//for each sample...
		buf0 = buf0 + f * (in - buf0 + fb * (buf0 - buf1));
		buf1 = buf1 + f * (buf0 - buf1);

		psamples[i] = (float) buf1;
	}

	*b0 = buf0;
	*b1 = buf1;
}