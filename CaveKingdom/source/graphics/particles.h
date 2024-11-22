#pragma once

#include <SDL.h>

#include "graphics/vector.h"

typedef struct Particle {
    float x;
    float y;
    int size;
    int finish_tick;
    float weight;
    Vector2f velocity;
    SDL_Color color;
} Particle;

typedef struct ParticleListItem {  
    Particle particle;
    struct ParticleListItem* previous_list_item;  
    struct ParticleListItem* next_list_item;
} ParticleListItem;

typedef struct ParticleManager {
    int number_of_particles;
    ParticleListItem* first_particle;
} ParticleManager;

extern ParticleManager PARTICLE_MANAGER;

void new_particle(int x, int y, SDL_Color color);
void update_particles();
