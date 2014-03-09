#include "wavread.h"

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
	if(this->header.main_chunk.RIFF[0] != 'R' || this->header.main_chunk.RIFF[1] != 'I' || this->header.main_chunk.RIFF[2] != 'F') throw 2; //<- endianessre nem vagyunk tekintettel
    // egyelore nem foglalkozunk vele

	fread(&this->header.WAVE, 4, 1, this->infile);
    if(this->header.WAVE[0] != 'W' || this->header.WAVE[1] != 'A' || this->header.WAVE[2] != 'V' || this->header.WAVE[3] != 'E') throw 2;

	fread(&this->header.fmt_chunk, sizeof(this->header.fmt_chunk), 1, this->infile);
	if(this->header.fmt_chunk.fmt [0] != 'f' || this->header.fmt_chunk.fmt [1] != 'm' || this->header.fmt_chunk.fmt [2] != 't' || this->header.fmt_chunk.fmt [3] != ' ') throw 2;
    // tarolt audio tipusa, ha nem 1, akkor valamilyen tomoritest hasznal -> nem tudunk vele mit kezdeni
    if(this->header.fmt_chunk.audioFormat != 1) throw 3;

	fread(&this->header.extraParamSize, sizeof(this->header.extraParamSize), 1, this->infile);
	if (this->header.extraParamSize>0) fseek(this->infile, this->header.extraParamSize, SEEK_CUR);

	fread(&this->header.data_chunk, sizeof(this->header.data_chunk), 1, this->infile);

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

    //this->bufsize = AUDIO_BUFFER_LEN * this->header.fmt_chunk.numOfChan * (this->header.fmt_chunk.bitsPerSample/8);
	this->bufsize = AUDIO_BUFFER_LEN * this->header.fmt_chunk.blockAlign;
    //this->buffer = new char*[2];
    this->buffer[0] = new char[this->bufsize];
    this->buffer[1] = new char[this->bufsize];
	this->buffer[2] = new char[this->bufsize];

    //this->activeBuffer = 0;

    this->bytesRead = 0;
	this->streamlen = header.data_chunk.subchunk2Size;

    this->_isEndOfStream = 0;

    this->frontBuffer = NULL;
	this->backBuffer = NULL;
    this->readBuffer = this->buffer[2];

}

WavRead::~WavRead()
{
    if (this->buffer[0] && this->buffer[1]) {
        delete this->buffer[0];
        delete this->buffer[1];
    }
}

void WavRead::fillBuffer(){
    if(!infile) throw 1;
    if(this->_isEndOfStream) throw 2;

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
   // printf("%d bytes read in %d block             \r", this->bytesRead, data_read);
}

void WavRead::swapBuffer(){
	void *tmp1 = this->frontBuffer;
	this->frontBuffer = this->backBuffer;
	this->backBuffer = this->readBuffer;
	this->readBuffer = tmp1;
}

void WavRead::fillBufferComplex(int size, int offset, float* buffer, channel_t channel){
}

void WavRead::fillBuffer(int size, int offset, void* buffer){
	if (!this->frontBuffer){
		this->frontBuffer =	this->buffer[0];
		this->backBuffer = this->buffer[1];

		this->fillBuffer();
		this->swapBuffer();
		this->fillBuffer();
		this->swapBuffer();
	}
	
	int _offset = offset - (this->bytesRead-2*bufsize);
	if (_offset<0) 
		throw 4;
	if (!buffer) 
		throw 2;
	if (_offset+size>2*bufsize) 
		throw 1;

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

	if(offset>=bufsize) {
		this->fillBuffer();
		this->swapBuffer();
	}
}
