#pragma once

#include <SDL_ttf.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern SDL_Texture* screen;
extern SDL_Texture* gui;
extern TTF_Font* font;
extern SDL_Texture* text;



typedef enum {
    ui_element_inventory_slot,

    number_of_ui_elements, // DO NOT USE AS UI ELEMENT!
} UIElement;


void draw_world();
void init_rendering();