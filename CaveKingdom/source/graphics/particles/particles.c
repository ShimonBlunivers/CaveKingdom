#include <stdlib.h>
#include <stdbool.h>

#include "particles.h"
#include "world/time.h"

Particle* new_particle(int x, int y, SDL_Color color) {
	int duration = 200;

	Particle* particle = malloc(sizeof(Particle));
	if (particle == NULL) return;

	particle->x = x;
	particle->y = y;

	particle->size = 100 + rand() % 50;
	particle->weight = 1 + rand() % 5;
	particle->velocity = 1 + rand() % 5;
	particle->finish_tick = graphic_tick + duration;

	particle->color = color;

	return particle;
}

bool update_particle(Particle* particle) {
	if (particle->finish_tick - graphic_tick <= 0) {
		free_particle(particle);
		return false;
	}

	particle->x += particle->velocity.x;
	particle->y += particle->velocity.y;
	particle->velocity.y += particle->weight;

	return true;
}


void free_particle(Particle* particle) {
	free(particle);
	particle = NULL;
}