#include <stdbool.h>

#include "camera.h"
#include "world/chunk.h"
#include "entities/entity.h"

Camera camera = { 0, 0, 1. , .1, 3. };

void update_camera() {
    camera.x = TILE_SIZE * camera.zoom * (0.5 + main_player->x) - 0.5 * SCREEN_WIDTH;
    camera.y = TILE_SIZE * camera.zoom * (0.5 + main_player->y) - 0.5 * SCREEN_HEIGHT;
}