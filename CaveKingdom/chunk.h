#pragma once

#include "vector.h"

#define MAP_WIDTH 64
#define MAP_HEIGHT 64

#define TILE_SIZE 60

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 720

#define PLAYER_VISION 13

//int tick = 0;

Vector2 from_screen_to_tile_coords(Vector2 screen_coords);