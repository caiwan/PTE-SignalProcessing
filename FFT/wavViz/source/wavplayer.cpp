#include "wavplayer.h"

namespace{
    SDL_AudioSpec format;

    void fill_audio(void *udata, Uint8 *stream, int len){
        WavRead *reader = (WavRead*)udata;

        if(reader->isEndOfStream()) return;

        reader->fillBuffer();
        reader->swapBuffer();

        SDL_MixAudio(stream, (const Uint8*)reader->getBuffer(), len, SDL_MIX_MAXVOLUME);
    }
}

int WavPlayer::InitAudio(WavRead* reader){
    format.freq = reader->getSamplingFreq();
    format.format = AUDIO_S16; //<- ezt resoveolni kell
    format.channels = reader->getChannels();
    format.callback = fill_audio;
    format.samples = AUDIO_BUFFER_LEN*reader->getChannels(); //<- .. ?
    format.userdata = reader;

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
