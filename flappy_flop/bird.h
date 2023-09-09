#include <SDL2/SDL.h>

typedef struct bird
{
    /* data */
    float x,y;
    float vx, vy;
    bool dead;
} bird;

#define GRAVITY 5.81f
#define AIR_FRICTION 0.9f
#define BOUNCE_FACTOR 0.95f

const SDL_Rect clip_rect_teal = {.x = 0, .y = 0, .w = 340, .h = 340};
const SDL_Rect clip_rect_enemy = {.x = 340, .y = 340, .w = 340, .h = 340};
//TODO: ewwww
SDL_Rect bird_draw_rect = {.x = 0, .y = 0, .w = 50, .h = 50};

void drawBird(SDL_Renderer *rend, SDL_Texture * bird_tex, bird b) {
    bird_draw_rect.x = b.x;
    bird_draw_rect.y = b.y;
    SDL_RenderCopy(rend, bird_tex, &clip_rect_teal, &bird_draw_rect);
}

void drawEnemy(SDL_Renderer *rend, SDL_Texture * bird_tex, bird b) {
    bird_draw_rect.x = b.x;
    bird_draw_rect.y = b.y;
    SDL_RenderCopy(rend, bird_tex, &clip_rect_enemy, &bird_draw_rect);
}

void drawBirds(SDL_Renderer *rend, SDL_Texture *bird_tex, bird *birds, int amount) {
    for (int i = 0; i < amount; i++) {
        drawEnemy(rend, bird_tex, birds[i]);
    }
}

void update(bird *bird, float dt) {
    bird->vy += GRAVITY;
    float dvx = bird->vy * dt;
    bird->x += bird->vx * dt;
    bird->y += bird->vy * dt;

    // bounce from 'walls'
    if (bird->y > WINDOW_HEIGHT) {
        bird->vy *= -1.0f * BOUNCE_FACTOR;
        bird->y = WINDOW_HEIGHT;
    }
    if (bird->y < 0) {
        bird->vy *= -1.0 * BOUNCE_FACTOR;
        bird->y = 0;
    }
    if (bird->x > WINDOW_WIDTH) {
        bird->vx *= -1.f * BOUNCE_FACTOR;
        bird->x = WINDOW_WIDTH;
    }
    if (bird->x < 0) {
        bird->x *= -1.f * BOUNCE_FACTOR;
        bird->x = 0;
    }
}

void updateEnemies(bird *birds, int enemy_amount, bird *player, float dt) {
    for (int i = 0; i < enemy_amount; i++) {
        update(&birds[i], dt);
    }
}

