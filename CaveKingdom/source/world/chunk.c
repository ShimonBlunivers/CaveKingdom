#include <stdio.h>

#include "world/chunk.h"

#include "graphics/camera.h"
#include "world/perlin.h"
#include "entities/entity.h"

ChunkManager CHUNK_MANAGER;

bool add_chunk_to_manager(Chunk* chunk) {
	Chunk** new_chunks = realloc(CHUNK_MANAGER.chunks, sizeof(Chunk*) * (CHUNK_MANAGER.number_of_chunks + 1));
	if (new_chunks == NULL) {
		printf("Error: Failed to allocate memory for chunks array.\n");
		return false;
	}

	CHUNK_MANAGER.chunks = new_chunks;
	CHUNK_MANAGER.chunks[CHUNK_MANAGER.number_of_chunks] = chunk;
	CHUNK_MANAGER.number_of_chunks++;

	return true;
}

void reset_grid(Chunk* chunk) {
	for (int layer = 0; layer < number_of_height_layers; layer++) {
		for (int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT; i++) {
			if (layer == height_layer_ground) chunk->entity_list[i + CHUNK_WIDTH * CHUNK_HEIGHT * layer] = new_entity(entity_type_dirt, i % CHUNK_WIDTH, i / CHUNK_WIDTH);
			if (layer == height_layer_surface) chunk->entity_list[i + CHUNK_WIDTH * CHUNK_HEIGHT * layer] = new_entity(entity_type_surface_empty, i % CHUNK_WIDTH, i / CHUNK_WIDTH);
			if (layer == height_layer_air) chunk->entity_list[i + CHUNK_WIDTH * CHUNK_HEIGHT * layer] = new_entity(entity_type_air_empty, i % CHUNK_WIDTH, i / CHUNK_WIDTH);

			chunk->entity_position_grid[i][layer] = &chunk->entity_list[i + CHUNK_WIDTH * CHUNK_HEIGHT * layer];
		}
	}
}

Chunk* new_chunk(int x, int y) {
	Chunk* chunk = malloc(sizeof(Chunk));
	if (chunk == NULL) {
		printf("Error: chunk was NULL when creating new chunk.\n");
		return NULL;
	}
	chunk->x = x;
	chunk->y = y;

	reset_grid(chunk);

	add_chunk_to_manager(chunk);

	generate_world(chunk, rand() % 100000);

	return chunk;
}

bool init_chunk_manager() {
	CHUNK_MANAGER.number_of_chunks = 0;
	CHUNK_MANAGER.chunks = malloc(sizeof(Chunk*));

	create_edge_walls(new_chunk(0, 0));
	
	return true;
}


Chunk* get_chunk_from_global_position(int x, int y) {
	x /= CHUNK_WIDTH;
	y /= CHUNK_HEIGHT;
	for (int i = 0; i < CHUNK_MANAGER.number_of_chunks; i++) {
		Chunk* chunk = CHUNK_MANAGER.chunks[i];
		if (chunk->x == x && chunk->y == y) return chunk;
	}

	return NULL;
}

Chunk* get_chunk(int x, int y) {
	for (int i = 0; i < CHUNK_MANAGER.number_of_chunks; i++) {
		Chunk* chunk = CHUNK_MANAGER.chunks[i];

		if (chunk->x == x && chunk->y == y) return chunk;
	}

	return NULL;
}

Entity* get_entity_from_chunk(Chunk* chunk, int x, int y, HeightLayer layer) {
	x %= CHUNK_WIDTH;
	y %= CHUNK_HEIGHT;

	if (x < 0) x += CHUNK_WIDTH;
	if (y < 0) y += CHUNK_HEIGHT;

	return chunk->entity_position_grid[y * CHUNK_WIDTH + x][layer];
}


void free_chunk(Chunk* chunk) {
	for (int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT * number_of_height_layers; i++)
		free_entity(&chunk->entity_list[i]);
	free(chunk);
}

void generate_world(Chunk* chunk, int seed) {
	double freq = 10;
	double amp = 0.1;

	for (int y = 0; y < CHUNK_HEIGHT; y++) {
		for (int x = 0; x < CHUNK_WIDTH; x++) {
			if (!(x == 0 || y == 0 || x == CHUNK_WIDTH - 1 || y == CHUNK_HEIGHT - 1)) {
				int shifted_x = x + chunk->x * CHUNK_WIDTH;
				int shifted_y = y + chunk->y * CHUNK_HEIGHT;
				double noise = perlin((x + seed) * freq / CHUNK_WIDTH, (y + seed) * freq / CHUNK_HEIGHT) * amp;
				if (noise > 0) force_spawn_entity(new_entity(entity_type_stone, shifted_x, shifted_y));
				else force_spawn_entity(new_entity(entity_type_surface_empty, shifted_x, shifted_y));
			}
		}
	}
}

void create_edge_walls(Chunk* chunk) {
	for (int y = 0; y < CHUNK_HEIGHT; y++)
		for (int x = 0; x < CHUNK_WIDTH; x++)
			if (x == 0 || y == 0 || x == CHUNK_WIDTH - 1 || y == CHUNK_HEIGHT - 1) 
				force_spawn_entity(new_entity(entity_type_water, x + chunk->x * CHUNK_WIDTH, y + chunk->y * CHUNK_HEIGHT));
}

void free_world() {
	for (int chunk_index = 0; chunk_index < CHUNK_MANAGER.number_of_chunks; chunk_index++) {
		free_chunk(CHUNK_MANAGER.chunks[chunk_index]);
	}
}