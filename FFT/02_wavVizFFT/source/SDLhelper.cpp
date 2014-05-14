#include "SDLhelper.h"

namespace{
    SDL_AudioSpec format;

	int offset;

    void fill_audio(void *udata, Uint8 *stream, int len){
        try {
            WavRead *reader = (WavRead*)udata;
			
			if(reader->isEndOfStream()) return;
			if(reader->getBytesRead()<offset+len){
				reader->fillBuffer();
				reader->swapBuffer();
			}

            // SDL_MixAudio(stream, (const Uint8*)reader->getBuffer(), len, SDL_MIX_MAXVOLUME);
            reader->fillBuffer(len, offset, stream);

            offset += len;
        } catch (int e){
            printf("ERROR Except: %d\r\n", e); // itt ki kene lepni
        }
    }
}

int WavPlayer::InitAudio(WavRead* reader){
    format.freq = reader->getSamplingFreq();
	format.format = reader->getSDLAudioFormat();
    format.channels = reader->getChannels();
    format.callback = fill_audio;
    format.samples = AUDIO_BUFFER_LEN;
    format.userdata = reader;

	offset = 0;

    if ( SDL_OpenAudio(&format, NULL) < 0 ) {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return(-1);
    }

    return 0;
}

int WavPlayer::GetTimeSample(){
	return offset;
}


void WavPlayer::DestroyAudio(){
    SDL_CloseAudio();
}

void WavPlayer::PlayAudio(){
    SDL_PauseAudio(0);
}


//////////////////////////////////////////////////////////////////////////
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    if (x<0) return -1;
    if (y<0) return -1;

    if (x>=surface->w) return -1;
    if (y>=surface->h) return -1;

    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////////

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (x<0) return;
    if (y<0) return;

    if (x>=surface->w) return;
    if (y>=surface->h) return;

    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}