#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "wavread.h"
#include "FFT.h"

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
			inf = fopen(argv[1], "rb");
					isStdin = 0;
		}
		
		// outfile ha "-" akkor stdoutra ir
		if (strcmp("-", argv[2]) == 0){
			outf = stdout;
			isStdout = 1;
		} else {
			outf = fopen(argv[2], "wb");
			isStdout = 0;
		}
	}

	if (!inf || !outf) {printf("Cannot open file(s)"); return -1;}

	//////////////////////////////
	try {
		WavRead *reader = NULL; reader = new WavRead(inf);
		WavWrite *writer = NULL; 
		writer = new WavWrite(outf, reader->getSamplingFreq(), reader->getChannels(), reader->getLengthInSample(), reader->getIs8Bit());


		if (reader) delete reader;
	} catch (int e){
		printf("Cannot read file(s). Error code:%d", e);
	}

	if(!isStdin){fflush(inf);fclose(inf);}
	if(!isStdout){fflush(outf);fclose(outf);}

	return 0;
}