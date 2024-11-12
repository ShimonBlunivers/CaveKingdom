#include <stdbool.h>
#include <math.h>

#include "camera.h"
#include "world/chunk.h"
#include "entities/entity.h"

Camera camera = { 0, 0, 1.0, 0.1, 3.0 };

void update_camera() {
    double player_x = (double)main_player->x;
    double player_y = (double)main_player->y;

    if (main_player->tween != NULL) {
        Vector2 new_position = get_current_tween_position(*main_player->tween);
        player_x = (double)(new_position.x) / TILE_SIZE;
        player_y = (double)(new_position.y) / TILE_SIZE;
    }

    camera.x = (int)round(TILE_SIZE * camera.zoom * (0.5 + player_x) - 0.5 * SCREEN_WIDTH);
    camera.y = (int)round(TILE_SIZE * camera.zoom * (0.5 + player_y) - 0.5 * SCREEN_HEIGHT);
}