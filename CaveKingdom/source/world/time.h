#pragma once

#include <SDL_stdinc.h>

extern Uint32 game_tick;

extern Uint32 graphic_tick;
extern Uint32 last_updated_graphic_tick;
extern Uint32 last_updated_tick;

float delta_graphic_tick();