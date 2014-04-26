/// Wav file irasa es olvasasa
/**
    (Kozel) tetszoleses wav file irasara es olvasasara alkalmas osztalyok
*/

#ifndef WAVREAD_H
#define WAVREAD_H

#include <cstdio>
#include <SDL/SDL_audio.h>

#define AUDIO_BUFFER_LEN 16384

class WavRead;

////////////////////////////////////////////////////////////////////////////////////////
/// WAV fejlec
////////////////////////////////////////////////////////////////////////////////////////
struct WAV_header{
    struct {
		char RIFF[4];       ///< Magic number. "RIFF" (big-) vagy "RIFX" (little endian)
		int chunkSize;      ///< Cunk meret. *ez + 8 byte = teljes filemeret*
	} main_chunk;

    char WAVE[4];			///< "WAVE"

    /// Format chunk 'fmt-chunk'
    struct {
		char fmt[4];			///< "fmt " Subchunk 1 header
		int subchunk1Size;		///< subchunk merete *ez + 8 byte = subchunk merete*
		short int audioFormat;	///< Audio formatum. 1 ha tomoritetlen
		short int numOfChan;	///< Csatornak szama
		int samplesPerSec;		///< Mintaveteli frekvencia
		int bytesPerSec;		///< Egy masodpercnyi minta byteban.
		short int blockAlign;	///< Blokk meret. Egy sample (minden csatornara) merete byteban.
		short int bitsPerSample; ///< Egy minta savszelessege bitben egy csatornaban.
	} fmt_chunk;

	/// Extra parameter 
	/**
		Ezt kovetoen a wav tartalmazhat extra parmetereket
		'bug:' <br/>
		a format chunkot kovethetik extra infok; ennek meretet a format chunkot koveto 16 bites szo irja le <br/>
		a prolema, hogy nem minden wav ilyen, valamelyik ha nem tartalmaz ilyent nulla ertek szerepel <br/>
		valamelyikben pedig egybol kovetkezik a "data" string <br/>
		mivel standard inputrol is olvas, nincs lehetoseg a fileseekre, ezert olvasas kozben kell ezt eldonteni <br/>
	*/
	short int extraParamSize; // extra adatok, amiket at kell lepni

    /// Data subchunk
	struct{
		char subchunk2ID[4];    ///< "data"
		int subchunk2Size;		///< Adat hossza. *ez + 8 byte = adat chunk hossza*
	} data_chunk;
};

////////////////////////////////////////////////////////////////////////////////////////
/// Tetszoleges wav file olvasasara alkalmas osztaly
/*
	Korlatok:<br>
	- nem tamogat little endian formatumot.
	- nem tamogat tomoritett hangfileokat
	- csak 8 illetve 16 bites filet kepes olvasni
	- legfeljebb stereo lehet a stream
*/
////////////////////////////////////////////////////////////////////////////////////////
class WavRead
{
    private:
		struct WAV_header header;

		int isLocked, isFilled;

        int bufsize, streamlen, bytesRead;
        int _isEndOfStream, activeBuffer;
        void *buffer[3];  ///< triplabuffer
        void *frontBuffer, *backBuffer, *readBuffer;

		/// Forras stream
        FILE *infile;
	
	public:
		/// feltolti a buffer
		void fillBuffer();

		/// megcsereli a buffereket
        void swapBuffer();

    public:
		/// Konstruktor
		/**
			@param infile file stream pointer
		*/
        WavRead(FILE* infile);
        virtual ~WavRead();

        //inline void* getBuffer(){return this->frontBuffer;}
        //inline int getBufsize() {return this->bufsize;}

		/// Buffer elso byteja a stream elejehez kepest
		inline int getBufferStart(){return this->bytesRead?this->bytesRead - 2* this->bufsize : 0;}
		
		/// Buffer elso sampleje a stream elejehez kepst
		inline int getBufferStartSample(){return this->getBufferStart() / this->header.fmt_chunk.blockAlign;}

		/// Ennyi byte kerult olvasasra
		inline int getBytesRead(){return this->bytesRead;}

		/// Ennyi sample kerult olvasasra
		inline int getSamplesRead(){return this->bytesRead/this->header.fmt_chunk.blockAlign;}

		/// Tortent-e buffer iras az utolso olvasas ota
		inline int isBufferChanged(){if(this->isFilled){this->isFilled = 0; return 1;} return 0;}

		//inline int getFrontB
		/// Csatorna kivalasztas
		/** Csak stero csatornat tud */
		typedef enum channel_e{
			CH_MONO,	///< Mono
			CH_LEFT,	///< Bal
			CH_RIGHT	///< Jobb
		} channel_t;

		
		/// A buffer tartalmat a kivalasztott csatornabol felirja komplex alakban [-1..1] intervallumon
		void fillBufferComplex(float* buffer, channel_t channel);
		
		/// A buffer tartalmat a kivalasztott csatornabol felirja komplex alakban [-32768..32768] intervallumon
		void fillBufferComplex(int* buffer, channel_t channel);

		/// Buffer tartalmat kiirja raw formaban
		void fillBuffer(int size, int offset, void* buffer);

		/// mintaveteli frekvencia
        inline int getSamplingFreq(){return this->header.fmt_chunk.samplesPerSec;}

		/// csatornak szama
        inline int getChannels(){return this->header.fmt_chunk.numOfChan;}

		/// bitrata
        inline int getBitrate(){return this->header.fmt_chunk.bitsPerSample;}

		/// blokk meret
		inline int getBlockAlign(){return this->header.fmt_chunk.blockAlign;}

		/// stream teljes hossza sampleban
		inline int getLengthInSample(){return this->header.data_chunk.subchunk2Size / this->header.fmt_chunk.blockAlign;}

		/// 8 bites-e a stream
		inline int getIs8Bit(){return this->header.fmt_chunk.bitsPerSample == 8;}

#if 1 /*kihuz*/
#define K this->header.fmt_chunk.bitsPerSample
		/// felirja SDL_audio modulnak megfelo enumeratorral a minta tiousat
		/* @return AUDIO_S8 (signed 8bit) vagy AUDIO_S16 (signed 16 bit)*/
		inline int getSDLAudioFormat(){if (K == 8) return AUDIO_S8; else if (K == 16) return AUDIO_S16; return 0;}
#undef K
#endif /*kihuz*/

		/// megmondja, hogy veget ert-e a stream olvasasa
        inline int isEndOfStream(){return this->_isEndOfStream; }
};

////////////////////////////////////////////////////////////////////////////////////////
/// WAV file irasa
/**
	Korlatok:
	- nem tamogat little endian formatumot.
	- nem tamogat tomoritett hangfileokat
	- csak 8 illetve 16 bites filet kepes olvasni
	- legfeljebb stereo lehet a stream
*/
////////////////////////////////////////////////////////////////////////////////////////
class WavWrite{
	private:
		struct WAV_header header;
		FILE* outfile;

	public:
		/// Konstruktor
		/**
			@param outfile
			@param samplerate
			@param channel
			@param lengthInSample
			@param is8bit
		*/
		WavWrite(FILE* outfile, int samplerate, int channel, int lengthInSample, int is8bit = 0);

		/// Destruktor
		~WavWrite();

		/// 
		void writeComplex_old(float *data, int length);
};

#endif // WAVREAD_H
