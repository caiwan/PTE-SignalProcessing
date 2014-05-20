#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cmath>

#include "AudioProcesisng/FFT.h"
#include "AudioProcesisng/Filter.h"
#include "AudioProcesisng/wavread.h"

void print_usage(){
	fprintf(stderr, "Usage: filter.exe <sigma> <M> <infile> <outfile> \r\n");
}

int main(int argc, char **argv){
	int isStdin = 0, isStdout = 0;
	FILE *inf = NULL, *outf = NULL;

	int mvg = 0;
	float sigma = 0.;

	// filter in.wav out.wav

	if (argc<5){
		print_usage();
		return 1;
	} else {
		if (sscanf(argv[1], "%d", &mvg) == EOF) {
			print_usage();
			return 1;
		}
		
		if (sscanf(argv[2], "%f", &sigma) == EOF) { //<- itt a sigma nulla marad !!! 
			print_usage();
			return 1;
		}

		// infile ha "-" akkor stdintrol olvas
		if (strcmp("-", argv[3]) == 0){
			inf = stdin;
			isStdin = 1;
		} else {
			inf = fopen(argv[3], "rb");
			isStdin = 0;
		}
		
		// outfile ha "-" akkor stdoutra ir
		if (strcmp("-", argv[4]) == 0){
			outf = stdout;
			isStdout = 1;
		} else {
			outf = fopen(argv[4], "wb");
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
	Filter::Chain *chain = NULL;

	float *buf = NULL;

	try {
		// rw
		reader = new WavRead(inf);

		// ww
		writer = new WavWrite(outf, reader->getSamplingFreq(), reader->getChannels(), reader->getLengthInSample(), reader->getIs8Bit());

		// buf
		float *buffer[3] = {NULL, NULL, NULL};
		buffer[0] = new float [2*AUDIO_BUFFER_LEN];
		if (reader->getChannels() == 2) buffer[1] = new float [2*AUDIO_BUFFER_LEN];
		buffer[2] = new float [2*AUDIO_BUFFER_LEN];

		int tt = 0;
		
		//filt
		chain = new Filter::Chain(reader->getSamplingFreq());
		chain->setGenerator(new Filter::GeneratorWav(reader));
		chain->addChain(new Filter::Filter_FIR_Sinc(mvg, sigma, (float)reader->getSamplingFreq()));

		while (!reader->isEndOfStream() /*|| k--*/){

			reader->fillBuffer();
			reader->swapBuffer();

			chain->fillBufferFloat(buffer[0], AUDIO_BUFFER_LEN, WavRead::CH_LEFT);
			if (reader->getChannels() == 2) 
				chain->fillBufferFloat(buffer[1], AUDIO_BUFFER_LEN, WavRead::CH_RIGHT);

			writer->write(buffer[0], buffer[1], buffer[2], AUDIO_BUFFER_LEN, 1);
		};
		 
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