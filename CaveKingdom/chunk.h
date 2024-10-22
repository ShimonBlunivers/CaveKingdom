#pragma once

#include "vector.h"

#define CHUNK_WIDTH 64
#define CHUNK_HEIGHT 64

#define TILE_SIZE 60

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 720

#define PLAYER_VISION 13

extern int tick;

Vector2 from_screen_to_tile_coords(Vector2 screen_coords);