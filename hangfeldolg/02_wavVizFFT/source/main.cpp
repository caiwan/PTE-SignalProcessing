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

void drawBars(const complex *data, int samplerate, int samplelen, int x, int y, int w, int h, SDL_Surface *screen){
    SDL_Rect pos;
    pos.x = (screen->w*x)/100;
    pos.y = (screen->h*y)/100;
    pos.w = (screen->w*w)/100;
    pos.h = (screen->h*h)/100;

    int c = 0, vpos;
	unsigned char *cc = (unsigned char*)&c;
    float _samplerate = (float)samplerate;
    float freq = 0.0, ppos = 0.0, t = 0.0;
	float fft_freqstep = 1./(float) samplerate;
    float freqstep = 3./(float)pos.w; // 10^4 nagysagrendig megyunk

	float scale = .5*(float)pos.h/((float)(samplelen));
	//float scale = 1.;
	
    SDL_LockSurface(screen);

    for(int px=0; px<pos.w; px++){
        freq = 20*pow(10,(float)(px)*freqstep);
		ppos = (float)(samplelen-4) * freq * fft_freqstep;

		vpos = (int)floor(ppos);
		t = ppos - (float)vpos;

		if (vpos>=samplelen) return;

        float f0 = data[vpos+0].re * scale;
		float f1 = data[vpos+1].re * scale;
		float f2 = data[vpos+2].re * scale;
		float f3 = data[vpos+3].re * scale;

        float h = fabs(CubicInterpolate(f0, f1, f2, f3, t))*(float)pos.h;

        for(int py=0; py<pos.h; py++){
            if(py<h){
				cc[2] = (py * 255) / pos.h;
				cc[1] = 255 - (py * 255) / pos.h;
				cc[0] = 0x00;
				cc[3] = 0xff;
			}
            else c = 0x0;
            putpixel(screen, pos.x+px, pos.y+(pos.h-py), c);
        }
    }

    SDL_UnlockSurface(screen);
}

unsigned char *initDrawSpectogram( int x, int y, int w, int h, unsigned char*buffer, SDL_Surface *screen){
	SDL_Rect pos;
    pos.x = (screen->w*x)/100;
    pos.y = (screen->h*y)/100;
    pos.w = (screen->w*w)/100;
    pos.h = (screen->h*h)/100;

	unsigned char *bbuffer = buffer;

	if (!bbuffer){
		bbuffer = new unsigned char [pos.w*pos.h];
		memset(bbuffer, 0, pos.w*pos.h);
	} else {
		// realloc ... 
	}

	return bbuffer;
}

int drawSpectogram(const complex *data, int samplerate, int samplelen, int x, int y, int w, int h, unsigned char*buffer, int time, SDL_Surface *screen){
    SDL_Rect pos;
    pos.x = (screen->w*x)/100;
    pos.y = (screen->h*y)/100;
    pos.w = (screen->w*w)/100;
    pos.h = (screen->h*h)/100;

    int c = 0, vpos;
	
	unsigned char *cc = &buffer[pos.h*time];

    float _samplerate = (float)samplerate; // todo ...
    float freq = 0.0, ppos = 0.0, t = 0.0;
	float fft_freqstep = 1./(float) samplerate;
    float freqstep = 3./(float)pos.h; // 10^4 nagysagrendig megyunk

	float scale = 128./((float)(samplelen));

    for(int py=0; py<pos.h; py++){
        freq = 20.*pow(10,(float)(py)*freqstep);
		ppos = (float)(samplelen-4) * freq * fft_freqstep;

		vpos = (int)floor(ppos);
		t = ppos - (float)vpos;

		if (vpos>=samplelen) return (time+1)%pos.w;

        float f0 = data[vpos+0].re * scale, f1 = data[vpos+1].re * scale, f2 = data[vpos+2].re * scale, f3 = data[vpos+3].re * scale;
		float f = fabs(CubicInterpolate(f0, f1, f2, f3, t));
		f = (f>1.)?1.:f;
		f = (f<0.)?0.:f;
		cc[py] = (unsigned char)(f* 255);
    }

	SDL_LockSurface(screen);
	unsigned int color;
	cc = (unsigned char*)&color; int i; unsigned char k;
	for(int x = 0; x<pos.w; ++x)
		for(int y = 0; y<pos.h; ++y){
			i = x*pos.h+y; k = buffer[i];
			cc[0] = k; cc[2] = k; cc[1] = k;
			cc[3] = 0xff;
			putpixel(screen, pos.x+x, pos.y+(pos.h-y), color);
		}

    SDL_UnlockSurface(screen);

	return (time+1)%pos.w;
}

int main(int argc, char* argv[]){
    SDL_Surface *screen = NULL;

    SDL_Event event;

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	//screen = SDL_SetVideoMode(512, 320, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
    screen = SDL_SetVideoMode(1024, 640, 32, SDL_SWSURFACE | SDL_DOUBLEBUF /*| SDL_HWSURFACE*/);
	//screen = SDL_SetVideoMode(1920, 1080, 32, SDL_SWSURFACE | SDL_DOUBLEBUF |SDL_FULLSCREEN);

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
		unsigned char *spectogram1 = NULL, *spectogram2 = NULL;
		if (reader->getChannels() == 2){
			pos1.x = 0; pos1.y = 0;
			pos1.w = 100; pos1.h = 43;

			pos2.x = 0; pos2.y = 46;
			pos2.w = 100; pos2.h = 43;
			
			spectogram2 = initDrawSpectogram(pos2.x, pos2.y, pos2.w, pos2.h, spectogram2, screen);
		} else {
			pos1.x = 0; pos1.y = 0;
			pos1.w = 100; pos1.h = 70;
		}

		spectogram1 = initDrawSpectogram(pos1.x, pos1.y, pos1.w, pos1.h, spectogram1, screen);


		//reader->fillBuffer();
        WavPlayer::InitAudio(reader);

        FFT* fft = new FFT(FFT_SAMPLE, reader->getSamplingFreq());

		WavPlayer::PlayAudio();

        bool running = true;
		unsigned int dtime = timer->getDeltaTimeMs(), ttime = 0, frame = 0, frametime = 0, frameskip = 0;
		unsigned int btime = 0, s1time = 0, s2time = 0;

		int offset = 0, getnext = 1;

		float *fft_buffer[2];
		fft_buffer[0]= new float[2*AUDIO_BUFFER_LEN];
		if (reader->getChannels() == 2) fft_buffer[1] = new float[2*AUDIO_BUFFER_LEN]; else fft_buffer[1] = NULL;

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
					if (getnext) reader->fillBufferComplex(fft_buffer[0], WavRead::CH_LEFT);
                    fft->calculate(&fft_buffer[0][offset]);
                    drawBars(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, 0, 85, 49, 15, screen);
					s1time = drawSpectogram(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, pos1.x, pos1.y, pos1.w, pos1.h, spectogram1, s1time, screen);

					if (getnext) reader->fillBufferComplex(fft_buffer[1], WavRead::CH_RIGHT);
					fft->calculate(&fft_buffer[1][offset]);
                    drawBars(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, 51, 85, 49, 15, screen);
					s2time = drawSpectogram(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, pos2.x, pos2.y, pos2.w, pos2.h, spectogram1, s2time, screen);

					getnext = 0;
                } else {
					if (getnext) reader->fillBufferComplex(fft_buffer[0], WavRead::CH_MONO);
                    fft->calculate(&fft_buffer[0][offset]);
                    drawBars(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, 0, 85, 100, 15, screen);
					s1time = drawSpectogram(fft->getLastResult(), reader->getSamplingFreq(), FFT_SAMPLE, pos1.x, pos1.y, pos1.w, pos1.h, spectogram1, s1time, screen);
					getnext = 0;
                }

            } catch (int e){
                // egyes esetekben lemarad a bufferrol, ezert el kell dobni 1-1 framet emiatt sajnos.
                if (e == 14 || e == 11 || e ==115)
                    frameskip ++ ;
                else throw e;
            }

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

			SDL_Flip(screen);
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


