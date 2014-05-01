#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "AudioProcesisng/FFT.h"
#include "AudioProcesisng/FreqFilter.h"
#include "AudioProcesisng/wavread.h"

int main(int argc, char **argv){
	int isStdin = 0, isStdout = 0;
	FILE *inf = NULL, *outf = NULL;

	// filter in.wav out.wav

	if (argc<3){
		isStdin = 1;
		isStdout = 1;
		inf = stdin;
		outf = stdout;
	} else {
		// infile ha "-" akkor stdintrol olvas
		if (strcmp("-", argv[1]) == 0){
			inf = stdin;
					isStdin = 1;
		} else {
			inf = fopen(argv[1], "r");
			//inf = fopen("g:\prog\kep_es_hang\trunk\FFT\test.wav", "r");
					isStdin = 0;
		}
		
		// outfile ha "-" akkor stdoutra ir
		if (strcmp("-", argv[2]) == 0){
			outf = stdout;
			isStdout = 1;
		} else {
			outf = fopen(argv[2], "w");
			isStdout = 0;
		}
	}

	if (!inf || !outf) {
		printf("Cannot open file(s)"); return -1;
	}

	//////////////////////////////
	WavRead *reader = NULL; 
	WavWrite *writer = NULL; 

	FFT *fft = NULL;
	FreqFilter::FilterChain *chain = NULL;

	float *buf = NULL;

	try {
		// rw
		reader = new WavRead(inf);

		// ww
		writer = new WavWrite(outf, reader->getSamplingFreq(), reader->getChannels(), reader->getLengthInSample(), reader->getIs8Bit());
		fft = new FFT(AUDIO_BUFFER_LEN, reader->getSamplingFreq());

		// buf
		int *buffer[2] = {NULL, NULL};
		buffer[0] = new int [2*AUDIO_BUFFER_LEN];
		if (reader->getChannels() == 2) buffer[1] = new int [2*AUDIO_BUFFER_LEN];

		while (!reader->isEndOfStream()){
			reader->fillBuffer();
			reader->swapBuffer();

			reader->fillBufferComplex(buffer[0], WavRead::CH_LEFT);
			if (reader->getChannels() == 2) 
				reader->fillBufferComplex(buffer[1], WavRead::CH_RIGHT);

			writer->write(buffer[0], buffer[1], AUDIO_BUFFER_LEN);
		}

	} catch (int e){
		printf("Cannot read file(s). Error code:%d", e);
	}

	// delete
	if (reader) delete reader;
	if (writer) delete writer;

	if (fft) delete fft;
	if (chain) delete chain;

	// fclose
	if(!isStdin){fflush(inf);fclose(inf);}
	if(!isStdout){fflush(outf);fclose(outf);}

	return 0;
}