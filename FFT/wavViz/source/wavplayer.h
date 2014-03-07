#ifndef WAVPLAYER_H
#define WAVPLAYER_H

#include <SDL/SDL_audio.h>
#include <SDL/SDL.h>
#include "wavread.h"

namespace WavPlayer{
    extern int InitAudio(WavRead* reader);
    extern void DestroyAudio();
    extern void PlayAudio();

    extern int GetTimeSample();
    extern int GetTimeMs();
};

class Timer{
	private:
		unsigned int tick;
	public:
		Timer(){tick=0;}
		unsigned int getDeltaTimeMs(){
			if(tick==0){tick=SDL_GetTicks(); return 0;} 
			unsigned int d = SDL_GetTicks()-tick; tick+=d; 
			return d;
		}
};

#endif // WAVPLAYER_H
