/************************************************************************************
    Kep- es hangfeldolgozas Beadando feladat
    2014. 02. 22.

    Tetszoleges WAV file FFT vizailizacioja ido fuggvenyeben

    A szoftver parameterbe egy valid wav file eleresi utvonalat varja, illetve
    amennyiben nincs megadott filenev, a standard inputrol varja az adatfolyamot.

    Hasznalat:
    $ vawViz sample.wav
    $ cat sample.wav | wavViz
    $ avconv -i sample.mp3 -f wav - | wavViz

    Keszitette: Sari Peter / SAPSAAP.PTE
************************************************************************************/

#include <SDL/SDL.h>

#include <cstdlib>
#include <cstdio>

#include "wavread.h"
#include "wavplayer.h"
#include "FFT.h"

#define FFT_SAMPLE 2048

Uint32 getpixel(SDL_Surface *surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

float CubicInterpolate(float y0,float y1, float y2,float y3, float mu){
   float a0,a1,a2,a3,mu2;

   mu2 = mu*mu;
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}

void drawBars(const complex *data, int samplerate, int samplelen, int x, int y, int w, int h, SDL_Surface *screen){
    SDL_LockSurface(screen);

    SDL_Rect pos;
    pos.x = (screen->w*x)/100;
    pos.y = (screen->h*y)/100;
    pos.w = (screen->w*w)/100;
    pos.h = (screen->h*h)/100;

    int c = 0, vpos;
    float _samplerate = (float)samplerate; // todo ...
    float freq = 0.0, ppos = 0.0;
    float freqstep = ((float)samplelen / (float)(samplerate*pos.w));

    for(int px=0; px<pos.w; px++){
        if (px>=samplelen) return;

        freq = (float)(px+1)*freqstep;
        ppos = logf(freq) * (float)samplelen;

        //vpos = (int)ppos;
		vpos = (int)px;

        if (vpos>samplelen) return;

        float f = data[vpos].re / ((float)(samplelen)*.25);
        float h = fabs(f)*(float)pos.h;
        for(int py=0; py<pos.h; py++){
            if(py<h)
            c = 0xFFFFFFFF;
            else c = 0x0;
            putpixel(screen, pos.x+px, pos.y+py, c);
        }
    }

    SDL_UnlockSurface(screen);
}

int main(int argc, char* argv[]){
    SDL_Surface *screen;

    SDL_Event event;

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE | SDL_DOUBLEBUF /*| SDL_HWSURFACE*/);

    printf("FFT WavViz by Caiwan^IR \r\n");

    FILE *fp = stdin;

	int isReadFromStdin = 0;

    if (argc>1){
        fp = fopen(argv[1],"rb");
        if (!fp) {
            printf("No such file");
            return 1;
        }
        printf("Loading input stream from file\r\n");
		isReadFromStdin = 0;
    } else {
        printf("Awaiting input stream from stdin\r\n");
		isReadFromStdin = 1;
    }

    try{
		Timer* timer = new Timer();
        WavRead *reader = new WavRead(fp);
        WavPlayer::InitAudio(reader);

        FFT* fft = new FFT(FFT_SAMPLE, reader->getSamplingFreq());

		WavPlayer::PlayAudio();

        bool running = true;

		unsigned int dtime = 0, ttime = timer->getDeltaTimeMs(), frame = 0, frametime = 0, frameskip = 0;

        SDL_Event event;
        while (running) {
            try {
                int offset = (ttime * reader->getSamplingFreq() / 1000);
                if(offset < reader->getBufferStartSample()) offset = reader->getBufferStartSample();

                if (reader->getChannels() == 2){
#if 0
                    reader->fillBufferComplex(FFT_SAMPLE, offset, (float*)fft->getInputBuffer(), WavRead::CH_LEFT);
                    fft->calculate();

                    drawBars(fft->getLastResult(), FFT_SAMPLE, 0, 0, 49, 90, screen);

                    reader->fillBufferComplex(FFT_SAMPLE, offset, (float*)fft->getInputBuffer(), WavRead::CH_RIGHT);
                    fft->calculate();

                    drawBars(fft->getLastResult(), FFT_SAMPLE, 51, 0, 49, 90, screen);
#else
                    reader->fillBufferComplex(FFT_SAMPLE, offset, (float*)fft->getInputBuffer(), WavRead::CH_MONO);
                    fft->calculate();

                    drawBars(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, 0, 0, 100, 90, screen);
#endif
                } else {
                    // mono
                }

            } catch (int e){
                // egyes esetekben lemarad a bufferrol, ezert el kell dobni 1-1 framet emiatt sajnos.
                if (e == 4)
                    frameskip ++ ;
                else throw e;
            }

            SDL_Flip(screen);

			//if(!isReadFromStdin){
			running = !reader->isEndOfStream();
			//}
			ttime += dtime = timer->getDeltaTimeMs();
			printf("time: %d.%d                 \r", ttime/1000, ttime%1000);

			// FRAPS!
			if (frametime>=1000){
				printf("FPS: %d skipped: %d                \r\n", frame, frameskip);
				frameskip = 0;
				frametime = 0;
				frame = 0;
			} else {
				frametime += dtime;
				frame++;
			}

            while (SDL_PollEvent(&event)) {
				/* GLOBAL KEYS / EVENTS */
                switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running &= false;
                        break;
                    default: break;
                    }
                    break;
                case SDL_QUIT:
                    running &= false;
                    break;
                }
            }
        }

        WavPlayer::DestroyAudio();

        delete reader;
		delete timer;
		delete fft;

    } catch (int e){
        printf("Could not open file. Error code: %d\r\n",e);
    }

    fclose(fp);

    return 0;
}
/////////////////////////////////////////////////////////////////////////
// Ezeket majd ki kell dobalni egy kulon fileba
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
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (x<0) return;
    if (y<0) return;

    if (x>=surface->w) return;
    if (y>=surface->h) return;

    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
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

