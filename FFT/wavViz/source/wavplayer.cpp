#include "wavplayer.h"

namespace{
    SDL_AudioSpec format;

	int offset;

    void fill_audio(void *udata, Uint8 *stream, int len){
        WavRead *reader = (WavRead*)udata;

        if(reader->isEndOfStream()) return;

        // SDL_MixAudio(stream, (const Uint8*)reader->getBuffer(), len, SDL_MIX_MAXVOLUME);
		reader->fillBuffer(len, offset, stream);
		offset += len;
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

void WavPlayer::DestroyAudio(){
    SDL_CloseAudio();
}

void WavPlayer::PlayAudio(){
    SDL_PauseAudio(0);
}
