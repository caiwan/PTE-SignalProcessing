/**
    Tetszoleges wav file beolvasasara alkalmas osztaly
*/

#ifndef WAVREAD_H
#define WAVREAD_H

#include <cstdio>
#include <SDL/SDL_audio.h>

#define AUDIO_BUFFER_LEN 4096

class WavRead;

class WavRead
{
    private:

        struct WAV_header{
            struct {
				char RIFF[4];       // "RIFF" (big-) vagy "RIFX" (little endian)
				int chunkSize;      // ez + 8 byte = teljes filemeret
			} main_chunk;

            char WAVE[4];       // "WAVE"

            // elso sub-chunk
            struct {
				char fmt[4];        // "fmt "
				int subchunk1Size;  // ez + 8 byte = subchunk merete
				short int audioFormat;
				short int numOfChan;
				int samplesPerSec;
				int bytesPerSec;
				short int blockAlign;
				short int bitsPerSample;    // ezt kovetoen jonnenek meg extra adatok, amiket at kellene lepni.
			} fmt_chunk;

			short int extraParamSize; // extra adatok, amiket at kell lepni

            // masodik subchunk
			struct{
				char subchunk2ID[4];    // "data"
				int subchunk2Size;  // ez + 8 byte = adat hossza
			} data_chunk;

        } header;

        int bufsize, streamlen, bytesRead;
        int _isEndOfStream, activeBuffer;
        void *buffer[3];  // triplabuffer
        void *frontBuffer, *backBuffer, *readBuffer;

        FILE *infile;

		void fillBuffer();
        void swapBuffer();

    public:
        WavRead(FILE* infile);
        virtual ~WavRead();

        //inline void* getBuffer(){return this->frontBuffer;}
        //inline int getBufsize() {return this->bufsize;}
		inline int getbytesRead(){return this->bytesRead;}
		inline int getSamplesRead(){return this->bytesRead/this->header.fmt_chunk.blockAlign;}
		
		//inline int getFrontB

		// retetek byteban ertendok
		void fillBuffer(int size, int offset, void* buffer);

        inline int getSamplingFreq(){return this->header.fmt_chunk.samplesPerSec;}
        inline int getChannels(){return this->header.fmt_chunk.numOfChan;}
        inline int getBitrate(){return this->header.fmt_chunk.bitsPerSample;}

#define K this->header.fmt_chunk.bitsPerSample
		inline int getSDLAudioFormat(){if (K == 8) return AUDIO_S8; else if (K == 16) return AUDIO_S16; return 0;}
#undef K
        inline int isEndOfStream(){return this->_isEndOfStream; }
};

#endif // WAVREAD_H
