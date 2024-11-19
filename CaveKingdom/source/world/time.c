#include "time.h"

Uint32 game_tick = 0;
Uint32 graphic_tick = 0;
Uint32 last_updated_tick = 0;

float delta_graphic_tick() {
	float delta_graphic_tick = (float)(SDL_GetTicks() - graphic_tick) / 1000;
	//printf("%f\n", delta_graphic_tick);
	return delta_graphic_tick;
}