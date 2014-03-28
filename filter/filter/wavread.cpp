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
	if (this->header.extraParamSize == 0x6164 || this->header.extraParamSize == 0x6461){
		fread(&this->header.extraParamSize, sizeof(this->header.extraParamSize), 1, this->infile);
		if (this->header.extraParamSize == 0x7461 || this->header.extraParamSize == 0x6174){
			fread(&this->header.data_chunk.subchunk2Size, sizeof(this->header.data_chunk.subchunk2Size), 1, this->infile); 
		}
	} else {
	
		if (this->header.extraParamSize>0) fseek(this->infile, this->header.extraParamSize, SEEK_CUR);
		fread(&this->header.data_chunk, sizeof(this->header.data_chunk), 1, this->infile);
	}

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
void WavRead::fillBufferComplex(float* buffer, channel_t channel){
	if (!this->frontBuffer) while(!this->frontBuffer); // ez is rossz megoldas
	if (this->isLocked) while(this->isLocked); 

	float c = 2./(float)(1<<this->header.fmt_chunk.bitsPerSample);

	int ch = this->header.fmt_chunk.numOfChan;
	int cs = (channel==CH_RIGHT && ch==2)?1:0;
	
	int bs = this->bufsize / this->header.fmt_chunk.blockAlign;

	if (this->header.fmt_chunk.bitsPerSample == 8){
		unsigned char *fbuf = (unsigned char*) this->frontBuffer, *bbuf = (unsigned char*)this->backBuffer;
		if (channel == CH_MONO && ch == 2){
			c *= .5;
			for(int i=0; i<bs; ++i){
				buffer[i              ] = .5 + c*((float)fbuf[ch*i+cs] + (float)fbuf[ch*i+cs]);
				buffer[bs+i] = .5 + c*((float)bbuf[ch*i+cs] + (float)bbuf[ch*i+cs]);
			}
		}else{
			for(int i=0; i<bs; ++i){
				buffer[i              ] = .5 + c * (float)fbuf[ch*i+cs];
				buffer[bs+i] = .5 + c * (float)bbuf[ch*i+cs];
			}
		}
	} else if (this->header.fmt_chunk.bitsPerSample == 16){
		unsigned short *fbuf = (unsigned short*) this->frontBuffer, *bbuf = (unsigned short*)this->backBuffer;
		if (channel == CH_MONO && ch == 2){
			c *= .5;
			for(int i=0; i<bs; ++i){
				buffer[i              ] = .5 + c*((float)fbuf[ch*i+cs] + (float)fbuf[ch*i+cs]);
				buffer[bs+i] = .5 + c*((float)bbuf[ch*i+cs] + (float)bbuf[ch*i+cs]);
			}
		}else{
			for(int i=0; i<bs; ++i){
				buffer[i              ] = .5 + c * (float)fbuf[ch*i+cs];
				buffer[bs+i] = .5 + c * (float)bbuf[ch*i+cs];
			}
		}
	}
}

// ez teljes egeszeben hibas
//void WavRead::fillBufferComplex(int size, int offset, float* buffer, channel_t channel){
//    this->isLocked = 1;
//	if (!this->frontBuffer){
//		return;
//		this->isLocked = 0;
//
//		this->frontBuffer =	this->buffer[0];
//		this->backBuffer = this->buffer[1];
//
//		this->fillBuffer();
//		this->swapBuffer();
//		this->fillBuffer();
//		this->swapBuffer();
//	}
//
//    int align = this->header.fmt_chunk.blockAlign;
//    int _size = size * align;
//    int _offset = offset * align - (this->bytesRead-2*bufsize);
//
//	if (_offset<0)
//		throw 4;
//	if (!buffer)
//		throw 2;
//	if (_offset+_size>2*bufsize)
//		throw 1;
//
//    int left = (_size+_offset)-this->bufsize;
//    float c = 1./(float)(1<<this->header.fmt_chunk.bitsPerSample);
//
//    int ch = this->header.fmt_chunk.numOfChan;
//    int cs = (channel==CH_RIGHT && ch==2)?1:0;
//
//    if(this->header.fmt_chunk.bitsPerSample == 8){
//
//    }
//    else if(this->header.fmt_chunk.bitsPerSample == 16){
//        unsigned short *bbuffer = (unsigned short*)((char*)this->frontBuffer+_offset);
//        if (left<0){
//            if (channel == CH_MONO && ch == 2){
//                c *= .5;
//                for (int i=0; i<size; i++){
//                    buffer[2*i+0] = c * (float)bbuffer[ch*i+cs] + c * (float)bbuffer[ch*i+cs] - .5;
//                    buffer[2*i+1] = 0.0f;
//                }
//            }
//            else
//                for (int i=0; i<size; i++){
//                    buffer[2*i+0] = c * (float)bbuffer[ch*i+cs] - .5;
//                    buffer[2*i+1] = 0.0f;
//                }
//        }else{
//            int bal_offset = _offset;
//            int bal_hossz = this->bufsize - bal_offset;
//            if (bal_hossz<0) bal_hossz = 0;
//
//            int bal_hossz_sample = bal_hossz / align;
//
//            int jobb_offset = _offset - this->bufsize;
//            int jobb_hossz = _size - bal_hossz;
//
//			if (jobb_offset<0) jobb_offset = 0;
//
//            int jobb_hossz_sample = jobb_hossz / align;
//
//            //memcpy(&((char*)buffer)[0], &((char*)this->frontBuffer)[bal_offset], bal_hossz);
//            if (channel == CH_MONO && ch == 2){
//                c *= .5;
//                bbuffer = (unsigned short*)((char*)this->frontBuffer+bal_offset);
//                if (bal_hossz > 0)
//                    for (int i=0; i<bal_hossz_sample; i++){
//                        buffer[2*i+0] = c * (float)bbuffer[ch*i+cs] + c * (float)bbuffer[ch*i+cs] - .5;
//                        buffer[2*i+1] = 0.0f;
//                    }
//
//                bbuffer = (unsigned short*)((char*)this->backBuffer+jobb_offset);
//                if (jobb_hossz>0)
//                    for (int i=0; i<jobb_hossz_sample; i++){
//						float k = c * (float)bbuffer[ch*i+1] + c * (float)bbuffer[ch*i+0] - .5;
//                        buffer[2*(bal_hossz_sample+i)+0] = k;
//                        buffer[2*(bal_hossz_sample+i)+1] = 0.0f;
//                    }
//            }
//            else{
//                bbuffer = (unsigned short*)((char*)this->frontBuffer+bal_offset);
//                if (bal_hossz > 0)
//                    for (int i=0; i<bal_hossz_sample; i++){
//                        buffer[2*i+0] = c * (float)bbuffer[ch*i+cs]- .5;
//                        buffer[2*i+1] = 0.0f;
//                    }
//
//                bbuffer = (unsigned short*)((char*)this->backBuffer+jobb_offset);
//                if (jobb_hossz>0)
//                    for (int i=0; i<jobb_hossz_sample; i++){
//                        buffer[2*(bal_hossz_sample+i)+0] = c * (float)bbuffer[ch*i+cs] - .5;
//                        buffer[2*(bal_hossz_sample+i)+1] = 0.0f;
//                    }
//
//        }   }
//    }
//    else throw 5;
//
//	this->isLocked = 0;
//}

void WavRead::fillBuffer(int size, int offset, void* buffer){
	//this->isLocked = 1;
	if (!this->frontBuffer){
		//this->isLocked = 0;

		this->frontBuffer =	this->buffer[0];
		this->backBuffer = this->buffer[1];

		this->fillBuffer();
		this->swapBuffer();
		this->fillBuffer();
		this->swapBuffer();
	}

	if (this->isLocked) while(this->isLocked); 

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

	//this->isLocked = 0;
	
	if(_offset>=bufsize) {
		this->fillBuffer();
		this->swapBuffer();
	}
}

/**********************************************************************************************************/

WavWrite::WavWrite(FILE* outfile, int samplerate, int channel, int lengthInSample, int is8bit){
	this->outfile = outfile;
	
	memset(&this->header, 0, sizeof(this->header));
	
	this->header.main_chunk.RIFF[0] = 'R';
	this->header.main_chunk.RIFF[1] = 'I';
	this->header.main_chunk.RIFF[2] = 'F';
	this->header.main_chunk.RIFF[3] = 'F';
	
	int block = (is8bit)?channel:2*channel; 
	int header = sizeof(this->header);

	this->header.main_chunk.chunkSize = block*lengthInSample + header - 8;

	this->header.WAVE[0] = 'W';
	this->header.WAVE[1] = 'A';
	this->header.WAVE[2] = 'V';
	this->header.WAVE[3] = 'E';

	this->header.fmt_chunk.fmt[0] = 'f';
	this->header.fmt_chunk.fmt[1] = 'm';
	this->header.fmt_chunk.fmt[2] = 't';
	this->header.fmt_chunk.fmt[3] = ' ';

	this->header.fmt_chunk.subchunk1Size = sizeof(this->header.fmt_chunk);

	this->header.data_chunk.subchunk2ID[0] = 'd';
	this->header.data_chunk.subchunk2ID[1] = 'a';
	this->header.data_chunk.subchunk2ID[2] = 't';
	this->header.data_chunk.subchunk2ID[3] = 'a';

	this->header.data_chunk.subchunk2Size = block * lengthInSample;

	fwrite(&this->header, sizeof(this->header), 1, this->outfile);
}

WavWrite::~WavWrite(){
	// ... 
}