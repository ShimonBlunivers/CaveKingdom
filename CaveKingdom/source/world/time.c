#include "time.h"

Uint32 game_tick = 0;
Uint32 graphic_tick = 0;
Uint32 last_updated_graphic_tick = 0;
Uint32 last_updated_tick = 0;

float delta_graphic_tick() {
	return (float)last_updated_graphic_tick / last_updated_tick;
}