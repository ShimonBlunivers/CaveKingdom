#include "time.h"

Uint32 game_tick = 0;
Uint32 graphic_tick = 0;
Uint32 last_updated_tick = 0;

Uint64 previous_counter;
Uint64 current_counter;
double delta_time = 0;

void update_time() {
    graphic_tick = SDL_GetTicks();
    current_counter = SDL_GetPerformanceCounter();
    delta_time = (double)(current_counter - previous_counter) / (double)SDL_GetPerformanceFrequency();
    previous_counter = current_counter;
}