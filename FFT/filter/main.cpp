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
		buf = new float[AUDIO_BUFFER_LEN];

		// ww
		writer = new WavWrite(outf, reader->getSamplingFreq(), reader->getChannels(), reader->getLengthInSample(), reader->getIs8Bit());

		// fft, fchain
		fft = new FFT(AUDIO_BUFFER_LEN, reader->getSamplingFreq());
		//chain = new FreqFilter::FilterChain(AUDIO_BUFFER_LEN, reader->getSamplingFreq());

		//chain->addFilter(new FreqFilter::FilterLowPass(0,0,0));

//		return 0;

		while (!reader->isEndOfStream()){

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