#include <stdbool.h>

#include "camera.h"
#include "chunk.h"
#include "entity.h"

Camera main_camera = { 0, 0, 1. , .1, 3. };

void update_camera() {
    main_camera.x = TILE_SIZE * main_camera.zoom * (0.5 + main_player->x) - 0.5 * SCREEN_WIDTH;
    main_camera.y = TILE_SIZE * main_camera.zoom * (0.5 + main_player->y) - 0.5 * SCREEN_HEIGHT;
}