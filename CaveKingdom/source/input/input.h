#pragma once

#include <stdbool.h>
#include <SDL_events.h>

#include "graphics/camera.h"
#include "graphics/vector.h"

typedef struct Mouse {
	int x;
	int y;
	bool left_button_pressed;
	bool right_button_pressed;
	bool left_button_clicked;
	bool right_button_clicked;
} Mouse;

extern Mouse mouse;

typedef enum KeyCode {
	key_w,
	key_s,
	key_a,
	key_d,

	key_f,

	key_1,
	key_2,
	key_3,
	key_4,
	key_5,
	key_6,
	key_7,
	key_8,
	key_9,
	key_0,

	key_tab,
	key_esc,

	number_of_keys, // DO NOT USE AS KEY CODE !
} KeyCode;


typedef struct Key {
	bool pressed;
	bool pressed_this_update;
	bool movement_key;
	Uint32 graphic_tick_pressed;
	Uint32 input_tick_pressed;
	SDL_KeyCode sdl_key_code;
} Key;

extern Key keyboard[number_of_keys];
extern Uint32 input_tick;

bool process_input();
bool key_tapped(Key key);
Vector2 from_screen_to_tile_coords(Vector2 screen_coords);
void init_input();

bool should_player_move_with_key(Key* key);