#pragma once
#include <stdbool.h>
#include <SDL_events.h>

#include "camera.h"

typedef struct {
    int x;
    int y;
    bool left_button_pressed;
    bool right_button_pressed;
    bool left_button_clicked;
    bool right_button_clicked;
} Mouse;

extern Mouse mouse;


typedef struct {
    bool w_key_pressed;
    bool s_key_pressed;
    bool a_key_pressed;
    bool d_key_pressed;
} Keyboard;

extern Keyboard keyboard;


bool process_input();