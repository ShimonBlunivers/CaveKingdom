#pragma once

#include "renderer.h"

extern SDL_Texture* ui_textures[number_of_ui_elements];
extern SDL_Texture* entity_textures[number_of_entity_types];
extern SDL_Texture* item_textures[number_of_item_types];

void load_textures();
void unload_textures();