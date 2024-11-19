#pragma once

#include <SDL.h>

extern Uint32 game_tick;

extern Uint32 graphic_tick;
extern Uint32 last_updated_graphic_tick;
extern Uint32 last_updated_tick;

extern Uint64 previous_counter;
extern Uint64 current_counter;
extern double delta_time;

void update_time();