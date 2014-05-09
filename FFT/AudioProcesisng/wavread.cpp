#include "wavread.h"

#include <Windows.h>

WavRead::WavRead(FILE* infile)
{
    if(!infile) throw 1;
    this->infile = infile;

    //Wav header feltoltese
    //fread(&this->header, sizeof(this->header), 1, this->infile);

	for(int i=0; i<sizeof(this->header); ++i) ((char*)&this->header)[i] = 0;

	// sajnos nem lehet a headert egy lepesben beolvasni :(
	fread(&this->header.main_chunk, sizeof(this->header.main_chunk), 1, this->infile);

    //Wav header ellenorzese
	if(this->header.main_chunk.RIFF[0] != 'R' || this->header.main_chunk.RIFF[1] != 'I' || this->header.main_chunk.RIFF[2] != 'F') throw 2; 
	// endianessre nem vagyunk tekintettel, ha "RIFF" helyett "RIFX" szerepel, akkor big endian kodolast hasznal

    // egyelore nem foglalkozunk vele, ha nem little endian, eldobjuk
	if (this->header.main_chunk.RIFF[2] != 'F')  throw 2; 

	fread(&this->header.WAVE, 4, 1, this->infile);
    if(this->header.WAVE[0] != 'W' || this->header.WAVE[1] != 'A' || this->header.WAVE[2] != 'V' || this->header.WAVE[3] != 'E') throw 2;

	fread(&this->header.fmt_chunk, sizeof(this->header.fmt_chunk), 1, this->infile);
	if(this->header.fmt_chunk.fmt [0] != 'f' || this->header.fmt_chunk.fmt [1] != 'm' || this->header.fmt_chunk.fmt [2] != 't' || this->header.fmt_chunk.fmt [3] != ' ') throw 2;
    
	// tarolt audio tipusa, ha nem 1, akkor valamilyen tomoritest hasznal -> nem tudunk vele mit kezdeni
    if(this->header.fmt_chunk.audioFormat != 1) throw 3;

	// jelenleg csak 8 illetve 16 bites mintakkal rendelkezo filet tudunk kezelni
	if(this->header.fmt_chunk.bitsPerSample != 8 && this->header.fmt_chunk.bitsPerSample != 16) throw 4;

	// //////////////////
	// bug:
	// a format chunkot kovethetik extra infok; ennek meretet a format chunkot koveto 16 bites szo irja le
	// a prolema, hogy nem minden wav ilyen, valamelyik ha nem tartalmaz ilyent nulla ertek szerepel
	// valamelyikben pedig egybol kovetkezik a "data" string
	// mivel standard inputrol is olvas, nincs lehetoseg a fileseekre, ezert olvasas kozben kell ezt eldonteni
	fread(&this->header.extraParamSize, sizeof(this->header.extraParamSize), 1, this->infile);
	if (this->header.extraParamSize == 0x6164 || this->header.extraParamSize == 0x6461){
		fread(&this->header.extraParamSize, sizeof(this->header.extraParamSize), 1, this->infile);
		if (this->header.extraParamSize == 0x7461 || this->header.extraParamSize == 0x6174){
			fread(&this->header.data_chunk.subchunk2Size, sizeof(this->header.data_chunk.subchunk2Size), 1, this->infile); 
		}
	} else {
	
		if (this->header.extraParamSize>0) fseek(this->infile, this->header.extraParamSize, SEEK_CUR);
		fread(&this->header.data_chunk, sizeof(this->header.data_chunk), 1, this->infile);
	}

#ifdef _DEBUG
#if 1 /*debug info*/
	printf("ChunkSize = %d \n", this->header.main_chunk.chunkSize);
	printf("Subchunk1Size (fmt) = %d\n", this->header.fmt_chunk.subchunk1Size);
	printf("Subchunk2Size (data) = %d\n", this->header.data_chunk.subchunk2Size);
	printf("AudioFormat = %d\n", this->header.fmt_chunk.audioFormat);
	printf("NumberOfChanels = %d\n", this->header.fmt_chunk.numOfChan);
	printf("SamplePerSec = %d\n", this->header.fmt_chunk.samplesPerSec);
	printf("BytePerSec = %d\n", this->header.fmt_chunk.bytesPerSec);
	printf("Bitrate = %d\n", this->header.fmt_chunk.bitsPerSample);
	printf("BlockAlignment = %d\n", this->header.fmt_chunk.blockAlign);
    printf("HeaderLength = %d\n", sizeof(this->header));	// <- block_alignment !
#endif /*debug info*/
#endif /*_DEBUG*/

	// bufferek feltoltese
	// harmat hasznal, egyikbe olvas, egyik az aktualis, egy pedig a tartalek.

    //this->bufsize = AUDIO_BUFFER_LEN * this->header.fmt_chunk.numOfChan * (this->header.fmt_chunk.bitsPerSample/8);
	this->bufsize = AUDIO_BUFFER_LEN * this->header.fmt_chunk.blockAlign;
    //this->buffer = new char*[2];
    this->buffer[0] = new char[this->bufsize];
    this->buffer[1] = new char[this->bufsize];
	this->buffer[2] = new char[this->bufsize];

    //this->activeBuffer = 0;

	//
	// adattagok inicializalasa

    this->bytesRead = 0;
	this->streamlen = header.data_chunk.subchunk2Size;

    this->_isEndOfStream = 0;

    this->frontBuffer = NULL;
	this->backBuffer = NULL;
    this->readBuffer = this->buffer[2];

	this->isLocked = 0;
	this->isFilled = 0;
}

WavRead::~WavRead()
{
    if (this->buffer[0] && this->buffer[1]) {
        delete this->buffer[0];
        delete this->buffer[1];
    }
}

void WavRead::fillBuffer(){
	if (!this->frontBuffer || !this->backBuffer){
		//this->isLocked = 0;

		this->frontBuffer =	this->buffer[0];
		this->backBuffer = this->buffer[1];

		this->fillBuffer();
		this->swapBuffer();
		this->fillBuffer();
		this->swapBuffer();

		return;
	}

    if(!infile) throw 1;
    if(this->_isEndOfStream) throw 2;

	//if (this->isLocked) while(this->isLocked); 
	this->isLocked = 1;

    int data_left = this->streamlen - this->bytesRead;
    int data_read = this->bufsize;
    if (data_left < this->bufsize) {
        data_read = data_left;
        this->_isEndOfStream = 1;    // stram vege
        //printf("End of audio stream\n");
    }

    fread(this->readBuffer, data_read, 1, this->infile);

    this->bufsize = data_read;
    this->bytesRead += data_read;
	
	this->isLocked = 0;
	this->isFilled = 1;
   // printf("%d bytes read in %d block             \r", this->bytesRead, data_read);
}

void WavRead::swapBuffer(){
	this->isFilled++;
	void *tmp1 = this->frontBuffer;
	this->frontBuffer = this->backBuffer;
	this->backBuffer = this->readBuffer;
	this->readBuffer = tmp1;
}

// visszaadja mindket buffer tartalmat teljes egeszeben
// 
void WavRead::fillBufferComplex(double* buffer, channel_t channel, int len, int is32k){
	if (!this->frontBuffer) while(!this->frontBuffer); // ez is rossz megoldas
	if (this->isLocked) while(this->isLocked); 

	double c = .5/(double)(1<<this->header.fmt_chunk.bitsPerSample);

	if (is32k) c = 1.;

	int ch = this->header.fmt_chunk.numOfChan;
	int cs = (channel==CH_RIGHT && ch==2)?1:0;
	
	int bs = this->bufsize / this->header.fmt_chunk.blockAlign;
	if (len && len<=bs) bs = len;

	// ezek itt nem jok!
	if (this->header.fmt_chunk.bitsPerSample == 8){
		unsigned char *fbuf = (unsigned char*) this->frontBuffer, *bbuf = (unsigned char*)this->backBuffer;
		if (channel == CH_MONO && ch == 2){
			c *= .5;
			for(int i=0; i<bs; ++i){
				buffer[i   ] = .5*c*((double)fbuf[ch*i+cs] + (double)fbuf[ch*i+cs]);
				if (!len) buffer[bs+i] = .5*c*((double)bbuf[ch*i+cs] + (double)bbuf[ch*i+cs]);
			}
		}else{
			for(int i=0; i<bs; ++i){
				buffer[i   ] = c * (double)fbuf[ch*i+cs];
				if (!len) buffer[bs+i] = c * (double)bbuf[ch*i+cs];
			}
		}
	} else if (this->header.fmt_chunk.bitsPerSample == 16){
		unsigned short *fbuf = (unsigned short*) this->frontBuffer, *bbuf = (unsigned short*)this->backBuffer;
		if (channel == CH_MONO && ch == 2){
			c *= .5;
			for(int i=0; i<bs; ++i){
				buffer[i   ] = ((double)fbuf[ch*i+cs] + (double)fbuf[ch*i+cs]) *.5*c;
				if (!len) buffer[bs+i] = ((double)bbuf[ch*i+cs] + (double)bbuf[ch*i+cs]) *.5*c;
			}
		}else{
			for(int i=0; i<bs; ++i){
				buffer[i   ] = (double)fbuf[ch*i+cs] * c;
				if (!len) buffer[bs+i] = (double)bbuf[ch*i+cs] * c;
			}
		}
	}
}


void WavRead::fillBuffer(int size, int offset, void* buffer){
	//this->isLocked = 1;

	if (this->isLocked) while(this->isLocked); 

	int _offset = offset - (this->bytesRead-2*bufsize);
	if (_offset<0)
		throw 14;
	if (!buffer)
		throw 12;
	if (_offset+size>2*bufsize)
		throw 11;

	int left = (size+_offset)-this->bufsize;
	if (left<=0){
		memcpy(buffer, (char*)this->frontBuffer+_offset, size);
	} else {
		int bal_offset = _offset;
		int bal_hossz = this->bufsize - bal_offset;
		if (bal_hossz<0) bal_hossz = 0;

		int jobb_offset = _offset - this->bufsize;
		int jobb_hossz = size - bal_hossz;

		if (bal_hossz > 0)
			memcpy(&((char*)buffer)[0], &((char*)this->frontBuffer)[bal_offset], bal_hossz);

		if (jobb_hossz>0)
			memcpy(&((char*)buffer)[bal_hossz], &((char*)this->backBuffer)[jobb_offset], jobb_hossz);

		//memset(buffer, 0, size);
	}

	//this->isLocked = 0;
	/*
	if(_offset>=bufsize) {
		this->fillBuffer();
		this->swapBuffer();
	}
	*/
}

/**********************************************************************************************************/

WavWrite::WavWrite(FILE* outfile, int samplerate, int channel, int lengthInSample, int is8bit){
	this->outfile = outfile;
	
	if(!this->outfile) throw 2;
	if(channel > 2) throw 3;

	memset(&this->header, 0, sizeof(this->header));
	
	// a wav file olyan, hogy elore kell ismerni a teljes adatsor meretet
	// nem lehet streaming formatumot csinalni beloe, mint mas formatumokbol
	// 

	//
	// 1) main chunk feltoltese
	this->header.main_chunk.RIFF[0] = 'R';	// ezz messze nem elegans modszer
	this->header.main_chunk.RIFF[1] = 'I';
	this->header.main_chunk.RIFF[2] = 'F';
	this->header.main_chunk.RIFF[3] = 'F';
	
	int block = (is8bit)?channel:2*channel; 
	int header = sizeof(this->header);

	this->header.main_chunk.chunkSize = block*lengthInSample + header - 8 - 2;
	
	//
	// 2) "WAVE"
	this->header.WAVE[0] = 'W';
	this->header.WAVE[1] = 'A';
	this->header.WAVE[2] = 'V';
	this->header.WAVE[3] = 'E';

	//
	// 3) format chunk feltoltese
	this->header.fmt_chunk.fmt[0] = 'f';
	this->header.fmt_chunk.fmt[1] = 'm';
	this->header.fmt_chunk.fmt[2] = 't';
	this->header.fmt_chunk.fmt[3] = ' ';

	this->header.fmt_chunk.subchunk1Size = sizeof(this->header.fmt_chunk) - 8 + 2;
	this->header.fmt_chunk.audioFormat = 1;					// csak PCM wavot vagyunk kepesek irni (format=1)
	this->header.fmt_chunk.bitsPerSample = is8bit?(8):(16); // csak 8 illetve 16 bites mintakkal tudunk dolgozni
	this->header.fmt_chunk.blockAlign = block;
	this->header.fmt_chunk.numOfChan = channel;
	this->header.fmt_chunk.samplesPerSec = samplerate;
	this->header.fmt_chunk.bytesPerSec = block * samplerate;

	//this->header.extraParamSize = 2; 

	//
	// 4) data chunk
	this->header.data_chunk.subchunk2ID[0] = 'd';
	this->header.data_chunk.subchunk2ID[1] = 'a';
	this->header.data_chunk.subchunk2ID[2] = 't';
	this->header.data_chunk.subchunk2ID[3] = 'a';

	this->header.data_chunk.subchunk2Size = block * lengthInSample;

	// 5) kiirjuk a kesz headert
	//fwrite(&this->header, sizeof(this->header), 1, this->outfile);
	// sajnos csak darabokba lehet a block alignment miatt
	fwrite(&this->header.main_chunk, sizeof(this->header.main_chunk), 1, this->outfile);
	fwrite(&this->header.WAVE, sizeof(this->header.WAVE), 1, this->outfile);
	fwrite(&this->header.fmt_chunk, sizeof(this->header.fmt_chunk), 1, this->outfile);

	fputc(0, this->outfile);
	fputc(0, this->outfile);

	fwrite(&this->header.data_chunk, sizeof(this->header.data_chunk), 1, this->outfile);

	
	fflush(this->outfile);
}

WavWrite::~WavWrite(){
	// ... 
}

//#define fclamp(x) (x<-1.?-1.:x>1.?1.:x) // nemmukodik
#define fclamp(x) (x)

void WavWrite::write(double *left, double *right, void* workbuffer, int length, int is32k){
	if (length<=0) throw 51;
	if (!left) throw 52;
	if (!right && this->header.fmt_chunk.numOfChan == 2) throw 53;

#if 0
	FILE *fp = fopen("teszt", "wb");
	if (!fp) return;
#else 
	FILE *fp = this->outfile;
#endif

	float c = 32768., k = 1.;
	if (is32k) c = 1., k = 32768.;

	if (this->header.fmt_chunk.bitsPerSample == 16){
		//unsigned short *buf = reinterpret_cast<unsigned short *>(workbuffer);	
		short *buf = reinterpret_cast<short *>(workbuffer);	
			if (this->header.fmt_chunk.numOfChan == 2){
				for (int i=0; i<length; i++){
					int
						v1 = (int)(fclamp(left[i ]) * c),
						v2 = (int)(fclamp(right[i]) * c);

					buf[2*i+0] = v1;
					buf[2*i+1] = v2;
				}
			} else if (this->header.fmt_chunk.numOfChan == 1){
				for (int i=0; i<length; i++){
					short v1 = (short)(fclamp(left[i]) * c);
					buf[i] = v1;
				}
			}
		}
	// TODO: 8 bitre is
	/*
	} else if (this->header.fmt_chunk.bitsPerSample == 8){
		unsigned char *buf = reinterpret_cast<unsigned char*>(workbuffer);
		for (int i=0; i<length; i++){
		if (this->header.fmt_chunk.numOfChan == 2){
	//			unsigned char 
	//				v1 = (unsigned char)(((left[i]  + 0x7fff) >> 8) & 0xff), 
	//				v2 = (unsigned char)(((right[i] + 0x7fff) >> 8) & 0xff);
	//			fwrite(&v1, 1, 1, this->outfile);
	//			fwrite(&v2, 2, 1, this->outfile);
			} else {
//				unsigned char v1 = (unsigned char)(((left[i]  + 0x7fff) >> 8) & 0xff);
	//			fwrite(&v1, 1, 1, this->outfile);
			}
		}
	}
	*/
	
	// ededeti buffer ki, flush the toilet-
	fwrite(workbuffer, this->header.fmt_chunk.blockAlign, length, fp);
	fflush(fp);
}

void WavWrite::writeComplex_old(double *data, int length){
	double v0 = 0.0, v1 = 0.0, vk = (double)((1<<this->header.fmt_chunk.bitsPerSample)-1);
	int vi1 = 0, vi0 = 0; 
	unsigned char vc; unsigned int vs;
	int bl = (this->header.fmt_chunk.bitsPerSample == 8)?1:2;

	for(int i=0; i<length; i++){
		if (this->header.fmt_chunk.numOfChan == 2){
			v0 = data[2*i+0]; vi0 = (int)(v0*vk);
			v1 = data[2*i+1]; vi1 = (int)(v1*vk);
			if (this->header.fmt_chunk.bitsPerSample == 8) {
				vc = (vi0 & 0xff); fwrite(&vc, 1, 1, this->outfile);
				vc = (vi1 & 0xff); fwrite(&vc, 1, 1, this->outfile);
			} else {
				vs = (vi0 & 0xffff); fwrite(&vs, 2, 1, this->outfile);
				vs = (vi1 & 0xffff); fwrite(&vs, 2, 1, this->outfile);
			}
			//fprintf(this->outfile, "%d %d ",vi0, vi1);
		}
		else{
			v0 = data[i]; vi0 = (int)(v0*vk);
			if (this->header.fmt_chunk.bitsPerSample == 8) {
				vc = (vi0 & 0xff); fwrite(&vc, 1, 1, this->outfile);
			} else {
				vs = (vi0 & 0xffff); fwrite(&vs, 2, 1, this->outfile);
			}
		}
	}
}