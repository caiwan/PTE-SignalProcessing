#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cmath>

#include "AudioProcesisng/FFT.h"
#include "AudioProcesisng/Filter.h"
#include "AudioProcesisng/wavread.h"

void print_usage(){
	fprintf(stderr, "Usage: filter.exe <M> <infile> <outfile> \r\n");
}

int main(int argc, char **argv){
	int isStdin = 0, isStdout = 0;
	FILE *inf = NULL, *outf = NULL;

	// filter M in.wav out.wav

	int mvg = 0;

	if (argc<4){
		print_usage();
		return 1;
	} else {
		if (sscanf(argv[1], "%d", &mvg) == EOF) {
			print_usage();
			return 1;
		}

		// infile ha "-" akkor stdintrol olvas
		if (strcmp("-", argv[2]) == 0){
			inf = stdin;
			isStdin = 1;
		} else {
			inf = fopen(argv[2], "rb");
			isStdin = 0;
		}
		
		// outfile ha "-" akkor stdoutra ir
		if (strcmp("-", argv[3]) == 0){
			outf = stdout;
			isStdout = 1;
		} else {
			outf = fopen(argv[3], "wb");
			isStdout = 0;
		}
	}

	if (!inf || !outf) {
		fprintf(stderr,"Cannot open file(s)"); return -1;
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
		chain->addChain(new Filter::Filter_FIR_MA(mvg));

		while (!reader->isEndOfStream() /*|| k--*/){

			reader->fillBuffer();
			reader->swapBuffer();

			chain->fillBufferFloat(buffer[0], AUDIO_BUFFER_LEN, WavRead::CH_LEFT);
			if (reader->getChannels() == 2) 
				chain->fillBufferFloat(buffer[1], AUDIO_BUFFER_LEN, WavRead::CH_RIGHT);

			writer->write(buffer[0], buffer[1], buffer[2], AUDIO_BUFFER_LEN, 1);
		};
		 
	} catch (int e){
		fprintf(stderr,"Cannot read file(s). Error code:%d", e);
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