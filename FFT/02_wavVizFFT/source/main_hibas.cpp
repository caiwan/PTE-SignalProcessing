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

#include "../AudioProcesisng/FFT.h"
#include "../AudioProcesisng/Filter.h"
#include "../AudioProcesisng/wavread.h"

#include "SDLhelper.h"

#define FFT_SAMPLE 16384

struct spectogram_object_t{
	unsigned char *screenBuffer;

	int x, y, w, h;
	SDL_Rect pos;

	int samplerate, samplelen;
	int time;
	
};

struct spectogram_object_t *initDrawSpectogram( int x, int y, int w, int h, int samplerate, int samplelen, struct spectogram_object_t * in_sobj, SDL_Surface *screen){
	struct spectogram_object_t *sobj = NULL;

	if (!in_sobj){
		// alloc
		sobj = new struct spectogram_object_t;
	
		unsigned char *bbuffer = sobj->screenBuffer;

		int _w = (screen->w*w)*(screen->h*h)/10000; //remelem jo

		bbuffer = new unsigned char [_w];
		memset(bbuffer, 0, _w);
	} else {
		// realloc
		sobj = in_sobj;
	}

	// set
	sobj->x = x; sobj->y = y; sobj->w = w; sobj->h = h;

    sobj->pos.x = (screen->w*sobj->x)/100;
    sobj->pos.y = (screen->h*sobj->y)/100;
    sobj->pos.w = (screen->w*sobj->w)/100;
    sobj->pos.h = (screen->h*sobj->h)/100;
	
	sobj->samplelen = samplelen;
	sobj->samplerate = samplerate;

	sobj->time = 0;

	return sobj;
}


int drawSpectogram(const complex *data, struct spectogram_object_t *sobj, SDL_Surface *screen){
    int c = 0, vpos;
	
	unsigned char *cc = &sobj->screenBuffer[sobj->pos.h*sobj->time];

    float _samplerate = (float)sobj->samplerate; // todo ...
    float freq = 0.0, ppos = 0.0, t = 0.0;
	float fft_freqstep = 1./(float) sobj->samplerate;
    float freqstep = 3./(float)sobj->pos.h; // 10^4 nagysagrendig megyunk

	float scale = 15./((float)(sobj->samplelen));

    for(int py=0; py<sobj->pos.h; py++){
        freq = 20.*pow(10,(float)(py)*freqstep);
		ppos = (float)(sobj->samplelen-4) * freq * fft_freqstep;

		vpos = (int)floor(ppos);
		t = ppos - (float)vpos;

		if (vpos>=sobj->samplelen) return (sobj->time+1)%sobj->pos.w;

        float f0 = data[vpos+0].re * scale, f1 = data[vpos+1].re * scale, f2 = data[vpos+2].re * scale, f3 = data[vpos+3].re * scale;
		float f = fabs(CubicInterpolate(f0, f1, f2, f3, t));
		f = (f>1.)?1.:f;
		f = (f<0.)?0.:f;
		cc[py] = (unsigned char)(f * 255);
    }

	SDL_LockSurface(screen);
	unsigned int color;
	cc = (unsigned char*)&color; int i; unsigned char k;
	for(int x = 0; x<sobj->pos.w; ++x)
		for(int y = 0; y<sobj->pos.h; ++y){
			i = x*sobj->pos.h+y; k = sobj->screenBuffer[i];
			cc[0] = k; cc[2] = k; cc[1] = k;
			cc[3] = 0xff;
			putpixel(screen, sobj->pos.x+x, sobj->pos.y+(sobj->pos.h-y), color);
		}

    SDL_UnlockSurface(screen);

	sobj->time = (sobj->time+1)%sobj->pos.w;
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct spectrum_object_t{
	//int x, y, w, h;
	SDL_Rect pos;

	int samplerate, samplelen;

	float *freq;	// adott pontokhoz tartozo frekvencia
	int *vpos;		// frekvencia szerinti pozicio a tombben
	float *ppos;	// interpolacio ket ertek kozott

	float scale;	// H scaling merteke
};

struct spectrum_object_t *initDrawBars(SDL_Rect rect, int samplerate, int samplelen, struct spectrum_object_t *in_sobj, SDL_Surface *screen){
	struct spectrum_object_t * sobj = NULL;

	if (!in_sobj){
		// alloc
		sobj = new struct spectrum_object_t;
		
		int _w = (screen->w*rect.w)/100;
		sobj->vpos = new int [_w];
		sobj->ppos = new float [_w];
		sobj->freq = new float [_w];

	} else {
		sobj = in_sobj;
		// ... realloc
	}

	// set
	//sobj->x = x; sobj->y = y; sobj->w = w; sobj->h = h;

    sobj->pos.x = (screen->w*rect.x)/100;
    sobj->pos.y = (screen->h*rect.y)/100;
    sobj->pos.w = (screen->w*rect.w)/100;
    sobj->pos.h = (screen->h*rect.h)/100;
	
	sobj->samplelen = samplelen;
	sobj->samplerate = samplerate;

	// rajzolashoz szuksees LUT generalasa
	float fft_freqstep = 2./(float) samplerate;	// 
    float freqstep = 3./(float)sobj->pos.w;		// 10^4 nagysagrendig megyunk

	for(int i=0; i<sobj->pos.w; i++){
		sobj->freq[i] = 20*pow(10,(float)(i)*freqstep);

		float fpos = (float)(samplelen-4) * sobj->freq[i] * fft_freqstep;

		sobj->ppos[i] = (int)floor(fpos);
		sobj->vpos[i] = fpos - (float)sobj->ppos[i];
	}

	return sobj;
}

void drawBars(const intComplex *data, struct spectrum_object_t *sobj, SDL_Surface *screen){
    int c = 0, vpos;
	unsigned char *cc = (unsigned char*)&c;
	
    SDL_LockSurface(screen);

    for(int px=0; px<sobj->pos.w; px++){
		int vpos = sobj->vpos[px];
		float t = sobj->ppos[px];

		if (vpos>=sobj->samplelen) return;

        float f0 = data[vpos+0].re; //* scale;
		float f1 = data[vpos+1].re; //* scale;
		float f2 = data[vpos+2].re; //* scale;
		float f3 = data[vpos+3].re; //* scale;

        float h = fabs(CubicInterpolate(f0, f1, f2, f3, t) / 65536.)*(float)sobj->pos.h;

        for(int py=0; py<sobj->pos.h; py++){
            if(py<h){
				cc[2] = (py * 255) / sobj->pos.h;
				cc[1] = 255 - (py * 255) / sobj->pos.h;
				cc[0] = 0x00;
				cc[3] = 0xff;
			}
            else c = 0x0;
            putpixel(screen, sobj->pos.x+px, sobj->pos.y+(sobj->pos.h-py), c);
        }
    }

    SDL_UnlockSurface(screen);
}

void destructDrawBars(struct spectrum_object_t *sobj){
	if (sobj){
		if(sobj->ppos) delete sobj->ppos;
		if(sobj->vpos) delete sobj->vpos;
		if(sobj->freq) delete sobj->freq;

		delete sobj;
	}
}

///////////////////////////////

int main(int argc, char* argv[]){
    SDL_Surface *screen = NULL;

    SDL_Event event;

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	//screen = SDL_SetVideoMode(512, 320, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
    screen = SDL_SetVideoMode(1024, 640, 32, SDL_SWSURFACE | SDL_DOUBLEBUF /*| SDL_HWSURFACE*/);
	//screen = SDL_SetVideoMode(1920, 1080, 32, SDL_SWSURFACE | SDL_DOUBLEBUF |SDL_FULLSCREEN);

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

		SDL_Rect pos1, pos2;
		//unsigned char *spectogram1 = NULL, *spectogram2 = NULL;
		struct spectogram_object_t *spectogram1 = NULL, *spectogram2 = NULL;
		struct spectrum_object_t   *spectrum1 = NULL, *spectrum2 = NULL;
		if (reader->getChannels() == 2){
			pos1.x = 0; pos1.y = 0;
			pos1.w = 100; pos1.h = 43;

			pos2.x = 0; pos2.y = 46;
			pos2.w = 100; pos2.h = 43;
			
			spectrum2 = initDrawBars(pos2, reader->getSamplingFreq(), FFT_SAMPLE, NULL, screen);

			//spectogram2 = initDrawSpectogram(pos2.x, pos2.y, pos2.w, pos2.h, spectogram2, screen);
		} else {
			pos1.x = 0; pos1.y = 0;
			pos1.w = 100; pos1.h = 70;
		}

		spectrum1 = initDrawBars(pos1, reader->getSamplingFreq(), FFT_SAMPLE, NULL, screen);
		//spectogram1 = initDrawSpectogram(pos1.x, pos1.y, pos1.w, pos1.h, spectogram1, screen);


		//reader->fillBuffer();
        WavPlayer::InitAudio(reader);

        FFT32* fft = new FFT32(FFT_SAMPLE, reader->getSamplingFreq());

		WavPlayer::PlayAudio();

        bool running = true;
		unsigned int dtime = timer->getDeltaTimeMs(), ttime = 0, frame = 0, frametime = 0, frameskip = 0;
		unsigned int btime = 0, s1time = 0, s2time = 0;

		int offset = 0, getnext = 1;

		int *fft_buffer[2];
		fft_buffer[0]= new int[2*AUDIO_BUFFER_LEN];
		if (reader->getChannels() == 2) fft_buffer[1] = new int[2*AUDIO_BUFFER_LEN]; else fft_buffer[1] = NULL;

        SDL_Event event;

        while (running) {
            try {
				if (reader->isBufferChanged()){ 
					btime = 0; getnext = 1;
				} else { 
					btime += dtime; getnext = 0;
				}

				offset = btime * reader->getSamplingFreq() / 1000;

				if (offset>AUDIO_BUFFER_LEN) 
					throw 115;

				//if (offset<0) throw 1;

                if (reader->getChannels() == 2){
					//if (getnext) reader->fillBufferComplex(fft_buffer[0], WavRead::CH_LEFT);
					if (getnext) reader->fillBufferComplex(fft_buffer[0], WavRead::CH_LEFT);
					fft->calculate(&fft_buffer[0][offset]);

                    //drawBars(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, 0, 85, 49, 15, screen);
					drawBars(fft->getLastResult(), spectrum1, screen);
					//s1time = drawSpectogram(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, pos1.x, pos1.y, pos1.w, pos1.h, spectogram1, s1time, screen);

					///
					if (getnext) reader->fillBufferComplex(fft_buffer[1], WavRead::CH_RIGHT);
					fft->calculate(&fft_buffer[1][offset]);

                    //drawBars(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, 51, 85, 49, 15, screen);
					drawBars(fft->getLastResult(), spectrum2, screen);
					//s2time = drawSpectogram(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, pos2.x, pos2.y, pos2.w, pos2.h, spectogram1, s2time, screen);

					getnext = 0;
                } else {
					if (getnext) reader->fillBufferComplex(fft_buffer[0], WavRead::CH_MONO);
                    fft->calculate(&fft_buffer[0][offset]);
                    //drawBars(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, 0, 85, 100, 15, screen);
					//s1time = drawSpectogram(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, pos1.x, pos1.y, pos1.w, pos1.h, spectogram1, s1time, screen);
					getnext = 0;
                }

            } catch (int e){
                // egyes esetekben lemarad a bufferrol, ezert el kell dobni 1-1 framet emiatt sajnos.
                if (e == 14 || e == 11 || e ==115)
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

        if (reader) delete reader;
		if (timer) delete timer;
		if (fft) delete fft;

		if (fft_buffer[0]) delete fft_buffer[0];
		if (fft_buffer[1]) delete fft_buffer[1];

    } catch (int e){
        printf("Could not open file. Error code: %d\r\n",e);
    }

    fclose(fp);

    return 0;
}


