#include "Filter.h"

// filter chain
Filter::Chain::Chain(int samplerate) : Generator(), filterChain(){
	this->samplerate = samplerate;
	this->sampletime = 1./(double)samplerate;

	this->buffer[0] = NULL;
	this->buffer[1] = NULL;
	this->generator = NULL;
}

void Filter::Chain::allocate(int len){
	if (!buffer[0] && len){
		//this->buffer[0] = malloc(len*sizeof(this->buffer[0]));
		this->buffer[0] = new double [len];
		this->buffer[1] = new double [len];
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

void Filter::Chain::render(int len, WavRead::channel_t channel){
	if (!this->generator) throw 21;
	//if (this->filterChain.empty()) throw 22;

	this->allocate(len);
	this->generator->fillBufferFloat(this->inbuf, len, channel);

	Filter *actfilter = NULL;

	for (int i=0; i<this->filterChain.size(); i++){
		actfilter = this->filterChain[i];
		
		if (actfilter) for(int p=0; p<this->bufsize; p++)
			actfilter->render(this->inbuf, this->outbuf, p);

		this->swapBuffers();
	}

	this->swapBuffers();
}

void Filter::Chain::fillBufferU16(unsigned short *data, int len, WavRead::channel_t channel){
	this->render(len, channel);
	throw 28;
}

void Filter::Chain::fillBufferInt(int *data, int len, WavRead::channel_t channel){
	this->render(len, channel);
	throw 28;
}

void Filter::Chain::fillBufferFloat(double *data, int len, WavRead::channel_t channel){
	this->render(len, channel);
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

	this->buffer[0] = new double[2*AUDIO_BUFFER_LEN];
	this->buffer[1] = new double[2*AUDIO_BUFFER_LEN];
	this->bpos = 0;
	//this->reader->fillBufferComplex(this->buffer, WavRead::CH_MONO);
}

void Filter::GeneratorWav::fillBufferFloat(double *data, int len, WavRead::channel_t channel){
	if (len <= AUDIO_BUFFER_LEN){
		//this->reader->fillBufferComplex(this->buffer[0], channel, 1);
		this->reader->fillBufferComplex(data, channel, len, 1);
		//memcpy(data, &this->buffer[0][0], len*sizeof(*data));
	} else {
		throw 25;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Filterek

void Filter::FilterRunningAvg:: render(double* inbuf, double* outbuf, int pos){
	double in = inbuf[pos];
	double res = in;
	if (pos)res = inbuf[pos] + inbuf[pos-1];
	outbuf[pos] = res *.5;
}


// 
void BmxResonanceFilter(double *psamples, int numsamples, double f, double q, double *b0, double *b1)
{
	double in, fb, buf0, buf1;

	buf0 = *b0;
	buf1 = *b1;

	fb = q + q / (1.0 - f);

	for (int i = 0; i < numsamples; i++)
	{
		in = psamples[i];

		//for each sample...
		buf0 = buf0 + f * (in - buf0 + fb * (buf0 - buf1));
		buf1 = buf1 + f * (buf0 - buf1);

		psamples[i] = (double) buf1;
	}

	*b0 = buf0;
	*b1 = buf1;
}