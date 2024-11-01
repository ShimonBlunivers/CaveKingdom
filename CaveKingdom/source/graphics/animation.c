#include <stdlib.h>

#include "animation.h"

#include "world/time.h"

float move_tile_tween_duration = 75;

Vector2 get_current_tween_position(Tween tween) {
	return (Vector2) {
		(int)((tween.finish_x - tween.start_x) * ((float)(graphic_tick - tween.start_tick) / (float)(tween.finish_tick - tween.start_tick))),
		(int)((tween.finish_y - tween.start_y) * ((float)(graphic_tick - tween.start_tick) / (float)(tween.finish_tick - tween.start_tick)))
	};
}

Tween* new_tween(int start_x, int start_y, int finish_x, int finish_y, int finish_tick) {
	Tween* tween = malloc(sizeof(Tween));
	if (tween != NULL) *tween = (Tween){ start_x, start_y, finish_x, finish_y, graphic_tick, finish_tick};
	return tween;
}

void delete_tween(Tween* tween) {
	free(tween);
}