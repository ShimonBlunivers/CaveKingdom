#include <stdlib.h>
#include <stdbool.h>

#include "particles.h"
#include "world/time.h"


ParticleManager PARTICLE_MANAGER = {
	.number_of_particles = 0,
	.first_particle = NULL,
};

void new_particle(int x, int y, SDL_Color color) {
	int duration = 300;
	ParticleListItem* new_item = malloc(sizeof(ParticleListItem));

	if (new_item == NULL) return;

	Particle particle;
	particle.x = x;
	particle.y = y;

	particle.size = 10;

	particle.x -= 0.5 * particle.size;
	particle.y -= 0.5 * particle.size;

	float multiplier = 5;

	particle.weight = 2500.;
	particle.velocity = (Vector2f){ ((float)(rand() % 51) - 25) * multiplier, -(20 + (float)(rand() % 60)) * multiplier };

	particle.finish_tick = graphic_tick + duration;
	particle.color = color;

	new_item->particle = particle;
	new_item->next_list_item = NULL;

	PARTICLE_MANAGER.number_of_particles++;
	if (PARTICLE_MANAGER.first_particle == NULL) {
		PARTICLE_MANAGER.first_particle = new_item;
	}
	else {
		ParticleListItem* item = PARTICLE_MANAGER.first_particle;
		while (item->next_list_item != NULL) {
			item = item->next_list_item;
		}
		item->next_list_item = new_item;
	}
}

void remove_particle_list_item(ParticleListItem* list_item) {
	if (list_item == NULL) return;

	if (PARTICLE_MANAGER.first_particle == list_item) {
		PARTICLE_MANAGER.first_particle = list_item->next_list_item;
	}

	PARTICLE_MANAGER.number_of_particles--;

	free(list_item);
}

bool update_particle_item(ParticleListItem* item) {
	if ((int) item->particle.finish_tick - (int) graphic_tick <= 0) {
		remove_particle_list_item(item);
		return false;
	}

	item->particle.x += item->particle.velocity.x * delta_time;
	item->particle.y += item->particle.velocity.y * delta_time;
	item->particle.velocity.y += item->particle.weight * delta_time;

	return true;
}

void update_particles() {
	ParticleListItem* item = PARTICLE_MANAGER.first_particle;
	while (item != NULL) {
		ParticleListItem* next_item = item->next_list_item;
		update_particle_item(item);
		item = next_item;
	}
}