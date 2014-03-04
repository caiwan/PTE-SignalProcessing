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

int main(int argc, char* argv[]){
    SDL_Surface *screen;

    SDL_Event event;

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);

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
        WavRead *reader = new WavRead(fp);
        WavPlayer::InitAudio(reader);
        WavPlayer::PlayAudio();

        bool running = true;

        SDL_Event event;
        while (running) {
            // do some fancy stuff here

			if(!isReadFromStdin){
				running = !reader->isEndOfStream();
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

    } catch (int e){
        printf("Could not open file. Error code: %d\r\n",e);
    }

    fclose(fp);

    return 0;
}

