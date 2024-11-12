#pragma once

#include <SDL.h>

#include "graphics/vector.h"

typedef struct {
    int x;
    int y;
    int size;
    int finish_tick;
    int weight;
    Vector2f velocity;
    SDL_Color color;
} Particle;