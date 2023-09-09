#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
//#include "lib/libSDL2.a"

#include <SDL2/SDL.h>

#include "config.h"
#include "bird.h"

// time / delta-time
#define TICK_INTERVAL 2
static Uint32 next_time;
int frameCounter = 0;

// Game objects...
#define BIRD_AMOUNT 200000
bird *birds;
int bi = 0;


// functions
Uint32 time_left();
void add_bird(bird *birds);
float rand_to(int max);


// to compile and run from the command line, run:
// gcc .\main.c -o main -I include -L lib -lmingw32 -lSDL2main -lSDL2; .\main
int main(int argc, char *argv[]) {
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window * window = SDL_CreateWindow(
        "flap-flop", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        SDL_WINDOW_SHOWN);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

    //SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_Surface *surf = SDL_LoadBMP("images/test_image.bmp");
    Uint32 colorkey = SDL_MapRGB(surf->format, 255, 255, 255);
    SDL_SetColorKey(surf, SDL_TRUE, colorkey);

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect crect = {.x = 0, .y = 0, .w = 340, 340};
    SDL_Rect fscreen_rect = {.x = 0, .y = 0, .w = 340, 340};
    SDL_Rect half_screen = {.x = WINDOW_WIDTH/2, .y = WINDOW_HEIGHT/2, .w = 50, .h = 50};

    bird player = {.x = 250, .y = 450};
    birds = (bird*)malloc(BIRD_AMOUNT*sizeof(bird));

    Uint32 ticks = SDL_GetTicks();
    Uint32 accum = 0;
    next_time = SDL_GetTicks() + TICK_INTERVAL;
    bool mouseButtonDown = false;
    Uint32 color = {0};
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    color = SDL_MapRGBA(format, 120,180,220,255);
    /*printf("Starting loopsies..\n");*/
    float dx = 0;
    float dy = 0;
    const float move_speed = 5.5f;
    srand(time(0)); //TODO: did SDL have a wrapper for rand..?

    // bird lifetime tracking stuff
    int free_indices[BIRD_AMOUNT] = {0};

    //SDL_RenderSetVSync(renderer, 1);
    while (!quit) {
        SDL_Event event;
        int x, y;
        dx = 0.f;
        dy = 0.f;
        bool moving = false;
        int count = 0;
        const uint8_t *state;
        // Poll event queue until all of them are handled
        while (SDL_PollEvent(&event) && !quit) {
            
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouseButtonDown = true;
            case SDL_MOUSEMOTION:
                x = event.button.x;
                y = event.button.y;
                break;
            case SDL_MOUSEBUTTONUP:
                mouseButtonDown = false;
                break;
            case SDL_KEYDOWN:
                
                break;
            default:
                break;
            }
        }
        frameCounter++;
        Uint32 newTicks = SDL_GetTicks();
        Uint32 dt = newTicks - ticks;
        ticks = newTicks;
        accum += dt;
        if (accum >= 1000)  {
            printf("FPS: %d\n", frameCounter);
            frameCounter = 0;
            accum = 0;
        }
       
        // get input:
        state = SDL_GetKeyboardState(&count);
        if (count > 0) {
            if (state[SDL_SCANCODE_A])
                dx -= 1.0f;
            if (state[SDL_SCANCODE_D])
                dx += 1.0f;
            if (state[SDL_SCANCODE_W])
                dy -= 1.0f;
            if (state[SDL_SCANCODE_S])
                dy += 1.0f;
            // quit on esc pressed
            if (state[SDL_SCANCODE_ESCAPE]) {
                quit = true;
                break;
            }

            if (state[SDL_SCANCODE_SPACE]) {
                add_bird(birds);
            }
        }
        moving = true;

        // update birds etc.
        float friction = moving ? 0.5f : 0.99f;
        player.x += dx * move_speed * friction;
        player.y += dy * move_speed * friction;
        //printf("DT: %d %f %f\n", dt, (float)dt, (float)dt/1000.0);
        updateEnemies(birds, bi, &player, ((float)dt)/1000.0f);


        // Draw loop
        SDL_RenderClear(renderer);
        int res = SDL_RenderCopy(renderer, tex, &crect, &half_screen);
        drawBird(renderer, tex, player);
        drawBirds(renderer, tex, birds, bi);

        //SDL_RenderDrawRects(renderer, rects, 4);
        //SDL_RenderFillRects(renderer, rects,4);
        //SDL_RenderDrawRectsF(renderer, rects, 4);

        SDL_RenderPresent(renderer);
        //SDL_GL_SwapWindow(window);
        // Using SDL_Delay, tick_left() and next_time to smooth out the frame rate, as we're not
        // VSYNCing
        SDL_Delay(time_left());
        next_time += TICK_INTERVAL;
    }

    //

    // Cleanup
    printf("Exiting, cleaning resources\n");
    
    // cleanup game objects
    free(birds);
    
    SDL_FreeFormat(format);

    // cleanup rendering..
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("Done.\n");
    return 0;
}

Uint32 time_left() {
    Uint32 now = SDL_GetTicks();
    if (next_time <= now) {
        return 0;
    }
    return next_time - now;
}

void add_bird(bird *birds) {
    if (bi >= BIRD_AMOUNT) return;
    bird b = {.x = rand_to(WINDOW_WIDTH), .y = rand_to(WINDOW_HEIGHT), .vx = 0.0f, .vy = 0.0f, .dead = false};
    birds[bi] = b;
    bi++;
}

float rand_to(int max) {
    int a = rand()%max;
    return (float)a;
}