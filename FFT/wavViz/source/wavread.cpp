#include "wavread.h"

WavRead::WavRead(FILE* infile)
{
    if(!infile) throw 1;
    this->infile = infile;

    //Wav header feltoltese
    fread(&this->header, sizeof(this->header), 1, this->infile);

    //Wav header ellenorzese
    if(this->header.RIFF[0] != 'R' || this->header.RIFF[1] != 'I' || this->header.RIFF[2] != 'F') throw 2; //<- endianessre nem vagyunk tekintettel
    // egyelore nem foglalkozunk vele
    if(this->header.WAVE[0] != 'W' || this->header.WAVE[1] != 'A' || this->header.WAVE[2] != 'V' || this->header.WAVE[3] != 'E') throw 2;
    if(this->header.fmt [0] != 'f' || this->header.fmt [1] != 'm' || this->header.fmt [2] != 't' || this->header.fmt [3] != ' ') throw 2;
    // tarolt audio tipusa, ha nem 1, akkor valamilyen tomoritest hasznal -> nem tudunk vele mit kezdeni
    if(this->header.audioFormat != 1) throw 3;

#if 1 /*debug info*/
    printf("ChunkSize = %d \n", this->header.chunkSize);
    printf("Subchunk1Size (fmt) = %d\n", this->header.subchunk1Size);
    printf("Subchunk2Size (data) = %d\n", this->header.subchunk2Size);
    printf("AudioFormat = %d\n", this->header.audioFormat);
    printf("NumberOfChanels = %d\n", this->header.numOfChan);
    printf("SamplePerSec = %d\n", this->header.samplesPerSec);
    printf("BytePerSec = %d\n", this->header.bytesPerSec);
    printf("Bitrate = %d\n", this->header.bitsPerSample);
    printf("BlockAlignment = %d\n", this->header.blockAlign);
    printf("HeaderLength = %d\n", sizeof(this->header));
#endif /*debug info*/

    this->bufsize = AUDIO_BUFFER_LEN * this->header.numOfChan * (this->header.bitsPerSample/8);
    //this->buffer = new char*[2];
    this->buffer[0] = new char[this->bufsize];
    this->buffer[1] = new char[this->bufsize];
    this->activeBuffer = 0;

    this->bytesRead = 0;
    this->streamlen = header.subchunk2Size;

    this->_isEndOfStream = 0;

    this->frontBuffer = NULL;
    this->readBuffer = this->buffer[0];

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
        printf("End of audio stream\n");
    }

    fread(this->readBuffer, data_read, 1, this->infile);

    this->bufsize = data_read;
    this->bytesRead += data_read;
    printf("%d bytes read in %d block             \r", this->bytesRead, data_read);
}

void WavRead::swapBuffer(){
    this->frontBuffer = this->buffer[this->activeBuffer];
    this->activeBuffer = ++this->activeBuffer & 1;
    this->readBuffer = this->buffer[this->activeBuffer];
}









