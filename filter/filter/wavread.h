/**
    Tetszoleges wav file beolvasasara alkalmas osztaly
*/

#ifndef WAVREAD_H
#define WAVREAD_H

#include <cstdio>
#include <SDL/SDL_audio.h>

#define AUDIO_BUFFER_LEN 16384

class WavRead;

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
};

class WavRead
{
    private:
		struct WAV_header header;

		int isLocked, isFilled;

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
		inline int getBufferStart(){return this->bytesRead?this->bytesRead - 2* this->bufsize : 0;}
		inline int getBufferStartSample(){return this->getBufferStart() / this->header.fmt_chunk.blockAlign;}
		inline int getBytesRead(){return this->bytesRead;}
		inline int getSamplesRead(){return this->bytesRead/this->header.fmt_chunk.blockAlign;}

		inline int isBufferChanged(){if(this->isFilled){this->isFilled = 0; return 1;} return 0;}

		//inline int getFrontB
		typedef enum channel_e{
			CH_MONO,
			CH_LEFT,
			CH_RIGHT
		} channel_t;

		// retetek byteban ertendok
		void fillBufferComplex(float* buffer, channel_t channel);
		//void fillBufferComplex(int sample, int offset, float* buffer, channel_t channel);
		void fillBuffer(int size, int offset, void* buffer);

        inline int getSamplingFreq(){return this->header.fmt_chunk.samplesPerSec;}
        inline int getChannels(){return this->header.fmt_chunk.numOfChan;}
        inline int getBitrate(){return this->header.fmt_chunk.bitsPerSample;}
		inline int getBlockAlign(){return this->header.fmt_chunk.blockAlign;}
		inline int getLengthInSample(){return this->header.data_chunk.subchunk2Size / this->header.fmt_chunk.blockAlign;}
		inline int getIs8Bit(){return this->header.fmt_chunk.bitsPerSample == 8;}

#if 0 /*kihuz*/
#define K this->header.fmt_chunk.bitsPerSample
		inline int getSDLAudioFormat(){if (K == 8) return AUDIO_S8; else if (K == 16) return AUDIO_S16; return 0;}
#undef K
#endif /*kihuz*/
        inline int isEndOfStream(){return this->_isEndOfStream; }
};

class WavWrite{
	private:
		struct WAV_header header;
		FILE* outfile;

	public:
		WavWrite(FILE* outfile, int samplerate, int channel, int lengthInSample, int is8bit = 0);
		~WavWrite();
};

#endif // WAVREAD_H