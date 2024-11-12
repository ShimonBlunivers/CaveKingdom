#include <stdlib.h>
#include <math.h>

#include "animation.h"
#include "world/time.h"

int move_tile_tween_duration = 225;

Vector2 get_current_tween_position(Tween tween) {
	return (Vector2) {
		.x = tween.start_x + (int)round(((tween.finish_x - tween.start_x) * ((double)(graphic_tick - tween.start_tick) / (double)(tween.finish_tick - tween.start_tick)))),
		.y = tween.start_y + (int)round(((tween.finish_y - tween.start_y) * ((double)(graphic_tick - tween.start_tick) / (double)(tween.finish_tick - tween.start_tick)))),
	};
}

Tween* new_tween(int start_x, int start_y, int finish_x, int finish_y, int finish_tick) {
	Tween* tween = malloc(sizeof(Tween));
	if (tween != NULL) *tween = (Tween){ start_x, start_y, finish_x, finish_y, graphic_tick, finish_tick };
	return tween;
}


void delete_tween(Tween* tween) {
	free(tween);
}

Tween change_finish_tween(Tween tween1, Vector2 new_finish_position) {
	Vector2 new_start_position = get_current_tween_position(tween1);

	tween1.start_x = new_start_position.x;
	tween1.start_y = new_start_position.y;

	tween1.finish_x = new_finish_position.x;
	tween1.finish_y = new_finish_position.y;

	tween1.start_tick = graphic_tick;

	tween1.finish_tick = (int)round(sqrt(pow(tween1.finish_tick - tween1.start_tick, 2) + pow(move_tile_tween_duration, 2))) + graphic_tick;

	return tween1;
}