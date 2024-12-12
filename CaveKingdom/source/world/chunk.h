#pragma once

#include "graphics/vector.h"
#include "entities/entity.h"

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 16

#define TILE_SIZE 60

#define PLAYER_VIEW_DISTANCE 10
#define PLAYER_VIEW_DENSITY 100

typedef struct Entity Entity;

typedef struct Chunk {
	bool visible;
	int x;
	int y;

	Entity entity_list[CHUNK_WIDTH * CHUNK_HEIGHT * number_of_height_layers];
	Entity* entity_position_grid[CHUNK_WIDTH * CHUNK_HEIGHT][number_of_height_layers]; // For quicker access through coordinates

} Chunk;


typedef struct ChunkManager {
	int number_of_chunks;
	Chunk** chunks;
} ChunkManager;

extern ChunkManager CHUNK_MANAGER;

bool init_chunk_manager();
Chunk* new_chunk(int x, int y);
Chunk* get_chunk(int x, int y);
Chunk* get_chunk_from_global_position(int x, int y);
void free_chunk(Chunk* chunk);
Entity* get_entity_from_chunk(Chunk* chunk, int x, int y, HeightLayer layer);

void reset_grid(Chunk* chunk);
void generate_world(Chunk* chunk, int seed);
void create_edge_walls(Chunk* chunk);
void free_world();

bool save_world(char* path);
bool load_world(char* path);