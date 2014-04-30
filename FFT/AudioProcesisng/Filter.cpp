#include "Filter.h"

// filter chain
Filter::Chain::Chain(int samplerate) : Generator(){
	this->samplerate = samplerate;
	this->sampletime = 1./(float)samplerate;

	this->buffer[0] = NULL;
	this->buffer[1] = NULL;
	this->generator = NULL;
}

void Filter::Chain::allocate(int len){
	if (!buffer[0]){
		this->buffer[0] = malloc(len*sizeof(this->buffer[0]));
		this->buffer[1] = malloc(len*sizeof(this->buffer[0]));
		this->bufsize = len;
	} else if (this->bufsize != len){
		free(this->buffer[0]);
		free(this->buffer[1]);
		this->buffer[0] = NULL;
		this->buffer[1] = NULL;
		this->allocate(len);
	}

	this->inbuf = buffer[0];
	this->outbuf = buffer[0];
}

void Filter::Chain::render(int len){
	if (!this->generator) throw 1;
	if (!this->filterChain.empty()) throw 2;

	this->allocate(len);
	this->generator->fillBufferFloat(this->inbuf, len);
	this->swapBuffers();

	for (int i=0; i<this->filterChain.size(); i++){
		this->filterChain[i]->
	}
}

void Filter::Chain::fillBufferU16(unsigned short *data, int len){
	this->render(len);

}

// WAV READER
Filter::GeneratorWav::GeneratorWav(WavRead *_reader) : Generator() {
	if (!reader) throw 1;
	this->reader = _reader;
	this->reader->fillBuffer();

	this->buffer = new float[AUDIO_BUFFER_LEN];
	this->bpos = 0;
	this->reader->fillBufferComplex(this->buffer, WavRead::CH_MONO);
}

void Filter::GeneratorWav::fillBufferFloat(float *data, int len){
	if (len < AUDIO_BUFFER_LEN){
		if(bpos + len < AUDIO_BUFFER_LEN){
			memcpy(data, &this->buffer[this->bpos], len*sizeof(*data));
			bpos += len;
		} else {
			int left = len - (AUDIO_BUFFER_LEN - bpos);
			if (bpos < AUDIO_BUFFER_LEN){
				memcpy(data, &this->buffer[this->bpos], (AUDIO_BUFFER_LEN - bpos)*sizeof(*data));
			} 
			this->reader->fillBuffer();
			this->reader->fillBufferComplex(this->buffer, WavRead::CH_MONO);

			memcpy(data, &this->buffer[0], (left)*sizeof(*data));

			bpos = left;
		} 
	} else {
		throw 5;
	}
}

// 
void BmxResonanceFilter(float *psamples, int numsamples, double f, double q, double *b0, double *b1)
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

		psamples[i] = (float) buf1;
	}

	*b0 = buf0;
	*b1 = buf1;
}