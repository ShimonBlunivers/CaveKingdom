#include <stdio.h>
#include <stdbool.h>

#include "input.h"
#include "core/game.h"
#include "world/chunk.h"
#include "world/time.h"
#include "entities/entity.h"

Mouse mouse = { 0, 0, false, false, false };
Key keyboard[number_of_keys];

Uint32 input_tick = 1;

void init_input() {
	for (KeyCode key_code = 0; key_code < number_of_keys; key_code++) {
		keyboard[key_code] = (Key){
			.pressed = false,
			.input_tick_pressed = 0,
			.sdl_key_code = get_sdl_key_code(key_code),
		};

		switch (key_code) {
		case key_w:
		case key_s:
		case key_a:
		case key_d:
			keyboard[key_code].movement_key = true;
			break;
		default:
			keyboard[key_code].movement_key = false;
			break;
		}
	}
}

static SDL_KeyCode get_sdl_key_code(KeyCode code) {
	switch (code) {
	case key_w:
		return SDLK_w;
	case key_s:
		return SDLK_s;
	case key_a:
		return SDLK_a;
	case key_d:
		return SDLK_d;
	case key_f:
		return SDLK_f;
	case key_1:
		return SDLK_1;
	case key_2:
		return SDLK_2;
	case key_3:
		return SDLK_3;
	case key_4:
		return SDLK_4;
	case key_5:
		return SDLK_5;
	case key_6:
		return SDLK_6;
	case key_7:
		return SDLK_7;
	case key_8:
		return SDLK_8;
	case key_9:
		return SDLK_9;
	case key_0:
		return SDLK_0;
	case key_tab:
		return SDLK_TAB;
	case key_esc:
		return SDLK_ESCAPE;
	default:
		return 0;
	}
}

// Returns the key code of the given SDL key code, if the key code is not found, returns number_of_keys
static KeyCode get_key_code(SDL_KeyCode sdl_key_code) {
	for (KeyCode key_code = 0; key_code < number_of_keys; key_code++) {
		if (keyboard[key_code].sdl_key_code == sdl_key_code) return key_code;
	}
	return number_of_keys;
}

static void key_press(KeyCode key) {
	if (!keyboard[key].pressed) {
		keyboard[key].input_tick_pressed = input_tick + 1;
		keyboard[key].graphic_tick_pressed = graphic_tick;
		keyboard[key].pressed = true;
		keyboard[key].pressed_this_update = false;
	}
}

bool key_tapped(Key key) {
	return key.input_tick_pressed == input_tick;
}

bool process_input() {
	bool quit = false;

	mouse.left_button_clicked = false;
	mouse.right_button_clicked = false;

	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) quit = true;
		else if (event.type == SDL_KEYDOWN) {
			KeyCode key_code = get_key_code(event.key.keysym.sym);
			if (key_code != number_of_keys) key_press(key_code);

			//else if (SDLK_e == event.key.keysym.sym) camera.zoom = SDL_clamp(camera.zoom + .1, camera.min_zoom, camera.max_zoom);
			//else if (SDLK_q == event.key.keysym.sym) camera.zoom = SDL_clamp(camera.zoom - .1, camera.min_zoom, camera.max_zoom);
			//else if (SDLK_r == event.key.keysym.sym) print_inventory(&main_player->inventory);
		}
		else if (event.type == SDL_KEYUP) {
			KeyCode key_code = get_key_code(event.key.keysym.sym);
			if (key_code != number_of_keys) keyboard[key_code].pressed = false;
		}
		else if (event.type == SDL_MOUSEMOTION) {
			SDL_GetMouseState(&mouse.x, &mouse.y);
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (SDL_BUTTON_LEFT == event.button.button) {
				mouse.left_button_pressed = true;
				mouse.left_button_clicked = true;
			}
			else if (SDL_BUTTON_RIGHT == event.button.button) {
				mouse.right_button_pressed = true;
				mouse.right_button_clicked = true;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONUP) {
			if (SDL_BUTTON_LEFT == event.button.button) mouse.left_button_pressed = false;
			else if (SDL_BUTTON_RIGHT == event.button.button) mouse.right_button_pressed = false;
		}
	}

	if (key_tapped(keyboard[key_f])) thermal_vision = !thermal_vision;

	input_tick++;

	return quit || keyboard[key_esc].pressed;
}

Vector2 from_screen_to_tile_coords(Vector2 screen_coords) {

	Vector2f screen_coords_f = vector2_to_f(screen_coords);

	screen_coords_f = vector2f_divide(screen_coords_f, (Vector2f) { camera.zoom, camera.zoom });

	Vector2f camera_f = vector2_to_f((Vector2) { camera.x, camera.y });

	Vector2f adjusted_coords = vector2f_sum(screen_coords_f, camera_f);

	Vector2f tile_size_f = { (float)(TILE_SIZE), (float)(TILE_SIZE) };

	Vector2f tile_coords_f = vector2f_divide(adjusted_coords, tile_size_f);

	return (Vector2) { floor(tile_coords_f.x), floor(tile_coords_f.y) };
}

bool should_player_move_with_key(Key* key) {
	if (!key->pressed || inventory_opened) return false;

	int double_move_prevention = 150;

	if (key->movement_key && !key->pressed_this_update) {
		key->pressed_this_update = true;
		return true;
	}
	if (key->graphic_tick_pressed == graphic_tick || graphic_tick - key->graphic_tick_pressed > double_move_prevention) {
		key->pressed_this_update = true;
		return true;
	}
	return false;
}