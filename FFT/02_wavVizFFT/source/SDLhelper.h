#ifndef WAVPLAYER_H
#define WAVPLAYER_H

#include <SDL/SDL_audio.h>
#include <SDL/SDL.h>
#include "../AudioProcesisng/wavread.h"

namespace WavPlayer{
    extern int InitAudio(WavRead* reader);
    extern void DestroyAudio();
    extern void PlayAudio();

    extern int GetTimeSample();
};

/////////////////////////////
extern Uint32 getpixel(SDL_Surface *surface, int x, int y);
extern void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

inline float CubicInterpolate(float y0,float y1, float y2,float y3, float mu){
   float a0,a1,a2,a3,mu2;

   mu2 = mu*mu;
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}

/////////////////////////////

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

#endif /*WAVPLAYER_H*/
