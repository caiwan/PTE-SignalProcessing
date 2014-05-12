#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cmath>

#include "AudioProcesisng/FFT.h"
#include "AudioProcesisng/Filter.h"
#include "AudioProcesisng/wavread.h"

int main(int argc, char **argv){
	FILE *outf = fopen("303.wav", "wb");

	if (!outf) return -1;

	//////////////////////////////
	WavRead *reader = NULL; 
	WavWrite *writer = NULL; 

	FFT *fft = NULL;
	Filter::Chain *chain = NULL;

	float *buf = NULL;

	try {
		int ww = 256;

		writer = new WavWrite(outf, 44100, 1, ww*AUDIO_BUFFER_LEN, 0);

		// buf

		float *buffer[2] = {NULL, NULL};
		buffer[0] = new float [2*AUDIO_BUFFER_LEN];
		buffer[1] = new float [2*AUDIO_BUFFER_LEN];

		int tt = 0;
		
		//filt
		chain = new Filter::Chain(reader->getSamplingFreq());
		chain->setGenerator(new Filter::GanaratorSaw(440, 44100, .5));
		chain->addChain(new Filter::Filter_Resonance(... ));

		while (ww--){

			chain->fillBufferFloat(buffer[0], AUDIO_BUFFER_LEN, WavRead::CH_LEFT);
			writer->write(buffer[0], buffer[1], buffer[2], AUDIO_BUFFER_LEN, 1);
		};
		 
	} catch (int e){
		printf("Cannot read file(s). Error code:%d", e);
	}

	// delete
	if (writer) delete writer;
	if (chain) delete chain;

	// fclose
	fflush(outf);
	fclose(outf);

	return 0;
}