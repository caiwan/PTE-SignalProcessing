#ifndef WAVPLAYER_H
#define WAVPLAYER_H

#include <SDL/SDL_audio.h>
#include "wavread.h"

namespace WavPlayer{
    extern int InitAudio(WavRead* reader);
    extern void DestroyAudio();
    extern void PlayAudio();

    extern int GetTimeSample();
    extern int GetTimeMs();
};

#endif // WAVPLAYER_H
