#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
//#include "lib/libSDL2.a"

#include <SDL2/SDL.h>

#define TICK_INTERVAL 2
static Uint32 next_time;

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

int SetPixelsToRandomColor(uint8_t *pixels, int n);
void UpdatePixelValues(uint8_t *pixels, int n);
Uint32 time_left();
void paintBrush(uint8_t *pixels, int x, int y, int brushSize);

int frameCounter = 0;


// To quickly compile and run from the command line, run:
// gcc .\main.c -o main -I include -L lib -lmingw32 -lSDL2main -lSDL2; .\main
int main(int argc, char *argv[]) {
    printf("SDL2 pixel rendering tests in C\n");
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window * window = SDL_CreateWindow(
        "SDL2 Pixel rendering tests", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        SDL_WINDOW_SHOWN|SDL_WINDOW_VULKAN);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

    //SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_RGBA8888);

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    //SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!tex)
        printf("PANIC:!\n");

    SDL_Rect a = {.h = 10, .w = 10, .x = 10, .y = 10};
    SDL_Rect b = {.h = 10, .w = 10, .x = 100, .y = 100};
    SDL_Rect c = {.h = 10, .w = 10, .x = 150, .y = 150};
    SDL_Rect d = {.h = 10, .w = 10, .x = 200, .y = 200};
    SDL_Rect rects[4] = {a,b,c,d};

    int pixelAmount = 4 * WINDOW_HEIGHT * WINDOW_WIDTH;
    uint8_t *pixels = (uint8_t*)malloc(pixelAmount * sizeof(uint8_t));
    SDL_Point p;
    printf("Trying to allocate points\n");
    SDL_Point *points = (SDL_Point*)malloc(WINDOW_WIDTH*WINDOW_HEIGHT * sizeof(SDL_Point));
    int pointAmount = WINDOW_HEIGHT*WINDOW_WIDTH;
    //uint8_t *locked_pixels = NULL;
    //SDL_FreeSurface(surface);

    // seed random number gens...
    srand(time(0));
    SetPixelsToRandomColor(pixels, pixelAmount);

    // copy pixel colors to points as a test...
    for (int i = 0; i < pointAmount; i += 1) {
        int y = i/WINDOW_WIDTH;
        int x = i%WINDOW_WIDTH;
        points[i].x = x;
        points[i].y = y;
    }

    Uint32 ticks = SDL_GetTicks();
    Uint32 accum = 0;
    next_time = SDL_GetTicks() + TICK_INTERVAL;
    bool mouseButtonDown = false;
    Uint32 color = {0};
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    color = SDL_MapRGBA(format, 120,180,220,255);
    printf("Starting loopsies..\n");
    while (!quit) {
        SDL_Event event;
        int x, y;
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
            }
        }
        
        // Update pixel data
        frameCounter++;
        Uint32 newTicks = SDL_GetTicks();
        Uint32 dt = newTicks - ticks;
        ticks = newTicks;
        accum += dt;

        // every 100th frame, print FPS and reset framecounter
        if (accum >= 1000)  {
            printf("FPS: %d\n", frameCounter);
            frameCounter = 0;
            accum = 0;
        }
        // Update pixel values...
        if (!mouseButtonDown) {
            //UpdatePixelValues(pixels, pixelAmount);
            SDL_FillRects(surface, rects, 5, color);
        } else {
            paintBrush(pixels, x, y, 10);
        }

        // ... and then update them to the buffer on GPU with LockTexture 
        /*int pitch = 0; // TODO: WTF is pitch?
        if (!SDL_LockTexture(tex, NULL, (void*)&pixels, &pitch)) {
            UpdatePixelValues(pixels, pixelAmount);
            SDL_UnlockTexture(tex);
        }*/
        
        int success = SDL_UpdateTexture(tex, NULL, pixels, WINDOW_WIDTH*sizeof(Uint32));
        if (success < 0) {
            printf("Texture update failed:\n");
            printf(SDL_GetError());
            quit = true;
        }

        // Draw loop
        SDL_RenderClear(renderer);
        int res = SDL_RenderCopy(renderer, tex, NULL, NULL);
        if (res == -1) {
            quit = true;
            printf("Copy to renderer failed!\n");
        }

        SDL_SetRenderDrawColor(renderer, 0,220,245,255);
        if (SDL_RenderDrawPoints(renderer, points, pointAmount/2) < 0) {
            printf("Error in drwaing points...\n");
        }
        SDL_SetRenderDrawColor(renderer, 244,220,120,255);
        SDL_RenderDrawRects(renderer, rects, 4);
        SDL_RenderFillRects(renderer, rects,4);
        
        //SDL_RenderDrawRectsF(renderer, rects, 4);

        SDL_RenderPresent(renderer);
        SDL_GL_SwapWindow(window);
        // Using SDL_Delay, tick_left() and next_time to smooth out the frame rate, as we're not
        // VSYNCing
        SDL_Delay(time_left());
        next_time += TICK_INTERVAL;
    }

    //

    // Cleanup
    // remember to free pixel array
    printf("Exiting, cleaning resources\n");
    free(pixels);
    free(points);
    SDL_FreeFormat(format);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("Done.\n");
    return 0;
}

uint8_t randByte() {
    if (rand() < 1)
        return 0x22;
    return 0x54;
}

void UpdatePixelValues(uint8_t *pixels, int n) {
    //uint8_t rval = rand();
    for (int i = 0; i < n; i += 4) {
        //uint8_t rval = abs(sin((double)frameCounter/100.0)) * 255;
        uint8_t rval = 0x22;
        pixels[i] = rval;
        pixels[i+1] = randByte();
        pixels[i+2] = rval;
        //pixels[i+3] = 0xFF;
    }
}

int SetPixelsToRandomColor(uint8_t *pixels, int n) {
    int rval = rand();
    printf("Random value: %d\n", rval);
    for (int i = 0; i < n; i += 4) {
        pixels[i] = rval;
        pixels[i+1] = 0xAE;
        pixels[i+2] = rval;
        pixels[i+3] = 0xFF;
    }
}

Uint32 time_left() {
    Uint32 now = SDL_GetTicks();
    if (next_time <= now) {
        return 0;
    }
    return next_time - now;
}

int getPixelIndex(int x, int y) {
    return 4 * (y*WINDOW_WIDTH + x);
}

void paintBrush(uint8_t *pixels, int x, int y, int brushSize) {
    int xmin = SDL_max(0, x - brushSize/2);
    int xmax = SDL_min(x+brushSize/2, WINDOW_WIDTH);
    int ymin = SDL_max(0, y - brushSize/2);
    int ymax = SDL_min(y+brushSize/2, WINDOW_HEIGHT);
    for (int y = ymin; y <= ymax; y++) {
        for (int x = xmin; x <= xmax; x++) {
            int i = getPixelIndex(x, y);
            pixels[i] = 0xF8;
			pixels[i+1] = 0xFF;
			pixels[i+2] = 0x45;
			pixels[i+3] = 0xFF;
        }
    }
}