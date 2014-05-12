#include "Filter.h"

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
	this->freq = freq, this->sampling = sampling;
}

Filter::GanaratorSaw::~GanaratorSaw(){}

void Filter::GanaratorSaw::fillBufferFloat(float *data, int len, int offset, WavRead::channel_t channel){
	float t0 = (float)offset / this->sampling;
	float ts = 1. / this->sampling;
	float ff = this->freq / this->sampling;
	for(int i=0; i<len; i++){
		
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Filterek
// FFI Moving Avg

void Filter::Filter_FFI_MA::reset(){

}

void Filter::Filter_FFI_MA:: render(float* inbuf, float* outbuf, int length){
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