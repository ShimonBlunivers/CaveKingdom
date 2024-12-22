#include <stdbool.h>
#include <math.h>
#include <SDL.h>

#include "core/game.h"
#include "camera.h"
#include "world/chunk.h"
#include "entities/entity.h"

Camera camera = {
	.x = 0,
	.y = 0,
	.zoom = 1.0,
	.min_zoom = 0.5,
	.max_zoom = 3.0,
};

void update_camera() {
	if (main_player == NULL || !main_player_alive) return;
	double player_x = (double)main_player->x;
	double player_y = (double)main_player->y;

	if (main_player->tween != NULL) {
		Vector2 new_position = get_current_tween_position(*main_player->tween);
		player_x = (double)(new_position.x) / TILE_SIZE;
		player_y = (double)(new_position.y) / TILE_SIZE;
	}

	camera.x = (int)round(TILE_SIZE * (0.5 + player_x) - (0.5 * SCREEN_WIDTH) / camera.zoom);
	camera.y = (int)round(TILE_SIZE * (0.5 + player_y) - (0.5 * SCREEN_HEIGHT) / camera.zoom);
}