/**
    Tetszoleges wav file beolvasasara alkalmas osztaly
*/

#ifndef WAVREAD_H
#define WAVREAD_H

#include <cstdio>

#define AUDIO_BUFFER_LEN 4096

class WavRead;
//class WavPlayer;

class Buffer{
    friend class WavRead;
    //friend class WavPlayer;

    private:
        char *buf[2];
        char *readBuffer, *primaryBuffer, *secondaryBuffer;

    public:

};

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
        void *buffer[2];  // duplabuffer
        void *frontBuffer, *readBuffer;

        FILE *infile;

    public:
        WavRead(FILE* infile);
        virtual ~WavRead();

        void fillBuffer();
        void swapBuffer();

        inline void* getBuffer(){return this->frontBuffer;}
        inline int getBufsize() {return this->bufsize;}

        inline int getSamplingFreq(){return this->header.fmt_chunk.samplesPerSec;}
        inline int getChannels(){return this->header.fmt_chunk.numOfChan;}
        inline int getBitrate(){return this->header.fmt_chunk.bitsPerSample;}

        inline int isEndOfStream(){return this->_isEndOfStream; }
};

#endif // WAVREAD_H
