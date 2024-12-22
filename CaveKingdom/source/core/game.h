#pragma once
#include <stdbool.h>

#define GAME_VERSION "0.0.7"

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 720

// Ratios 1:x

#define UI_RATIO 2
#define TILE_RATIO 3

#define TILE_SIZE (TILE_RATIO * 26) // * texture_size

extern bool thermal_vision;
extern bool inventory_opened;