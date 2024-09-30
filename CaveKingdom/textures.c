#include <SDL.h>

#include "renderer.h"
#include "inventory.h"
#include "entity.h"


SDL_Texture* ui_textures[number_of_ui_elements] = { 0 };
SDL_Texture* entity_textures[number_of_entity_types] = { 0 };
SDL_Texture* item_textures[number_of_item_types] = { 0 };


void load_textures() {
    // UI textures
    ui_textures[ui_element_inventory_slot] = IMG_LoadTexture(renderer, "./assets/textures/ui/inventory_slot.png");


    // Entity textures
        // Ground
    entity_textures[entity_type_water] = IMG_LoadTexture(renderer, "./assets/textures/tiles/water.png");
    entity_textures[entity_type_dirt] = IMG_LoadTexture(renderer, "./assets/textures/tiles/dirt.png");

    // Surface
    entity_textures[entity_type_player] = IMG_LoadTexture(renderer, "./assets/textures/tiles/player.png");
    entity_textures[entity_type_enemy] = IMG_LoadTexture(renderer, "./assets/textures/tiles/enemy.png");
    entity_textures[entity_type_zombie] = IMG_LoadTexture(renderer, "./assets/textures/tiles/zombie.png");
    entity_textures[entity_type_stone] = IMG_LoadTexture(renderer, "./assets/textures/tiles/stone.png");
    // Air


// Item textures
    item_textures[item_type_stone] = IMG_LoadTexture(renderer, "./assets/textures/items/stone.png");
    item_textures[item_type_zombie_meat] = IMG_LoadTexture(renderer, "./assets/textures/items/zombie_meat.png");
}

void unload_textures() {
    for (int i = 0; i < number_of_ui_elements; i++) {
        SDL_DestroyTexture(ui_textures[i]);
    }
    for (int i = 0; i < number_of_entity_types; i++) {
        SDL_DestroyTexture(entity_textures[i]);
    }
    for (int i = 0; i < number_of_item_types; i++) {
        SDL_DestroyTexture(item_textures[i]);
    }
}