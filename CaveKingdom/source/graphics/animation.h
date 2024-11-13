#pragma once

#include "graphics/vector.h"

typedef struct Tween {
    int start_x;
    int start_y;
    int finish_x;
    int finish_y;

    int start_tick;
    int finish_tick;
} Tween;


Vector2 get_current_tween_position(Tween tween);
Tween* new_tween(int start_x, int start_y, int finish_x, int finish_y, int finish_tick);
void delete_tween(Tween* tween);
Tween change_finish_tween(Tween tween1, Vector2 position);

extern int move_tile_tween_duration;