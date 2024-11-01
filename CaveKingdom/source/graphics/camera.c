#include <stdbool.h>

#include "camera.h"
#include "world/chunk.h"
#include "entities/entity.h"

Camera camera = { 0, 0, 1. , .1, 3. };

void update_camera() {
    float player_x = (float)main_player->x;
    float player_y = (float)main_player->y;

    if (main_player->tween != NULL) {
        Vector2 shift = get_current_tween_position(*main_player->tween);
        player_x = (float)(main_player->tween->finish_x - shift.x) / TILE_SIZE;
        player_y = (float)(main_player->tween->finish_y - shift.y) / TILE_SIZE;
    }

    camera.x = TILE_SIZE * camera.zoom * (0.5 + player_x) - 0.5 * SCREEN_WIDTH;
    camera.y = TILE_SIZE * camera.zoom * (0.5 + player_y) - 0.5 * SCREEN_HEIGHT;

 
}