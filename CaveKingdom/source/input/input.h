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

typedef enum KeyCodes {
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

    number_of_keys, // DO NOT USE AS KEY CODE !
} KeyCodes;


typedef struct Key {
    bool pressed; // Is the key held down?
    bool active; // Should the key be seen as pressed? E.g., if the key is pressed, but the game should act like it isn't.
    Uint32 tick_pressed;
    int key_code;
} Key;

extern Key keyboard[number_of_keys];

bool process_input();

Vector2 from_screen_to_tile_coords(Vector2 screen_coords);
void init_input();