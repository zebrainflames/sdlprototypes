#include <stdio.h>
#include <stdbool.h>
//#include "lib/libSDL2.a"

#include <SDL2/SDL.h>

// To quickly compile and run from the command line, run:
// gcc .\main.c -o main -I include -L lib -lmingw32 -lSDL2main -lSDL2; .\main

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

int main(int argc, char *argv[]) {
    printf("SDL2 pixel rendering tests in C");
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window * window = SDL_CreateWindow(
        "SDL2 Pixel rendering tests", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        0);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    while (!quit) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}