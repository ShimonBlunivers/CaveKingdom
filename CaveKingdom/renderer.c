#include <SDL.h>
#include <SDL_ttf.h>

#include "entity.h"
#include "chunk.h"
#include "camera.h"
#include "textures.h"


SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Texture* screen = NULL;
SDL_Texture* gui = NULL;
TTF_Font* font = NULL;
SDL_Texture* text = NULL;


void draw_world() {

    SDL_SetRenderTarget(renderer, screen);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderClear(renderer);

    Entity entity;

    SDL_Rect tile;
    SDL_Color tile_color;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            for (int layer = 0; layer < number_of_height_layers; layer++)
            {
                entity = *get_entity(x, y, layer);
                if (!is_empty_entity_type(entity.type)) {
                    tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };
                    if (entity_textures[entity.type] != NULL) {
                        printf("entity_textures[entity.type] = %p\n", entity_textures[entity.type]);
                        SDL_RenderCopy(renderer, entity_textures[entity.type], NULL, &tile);
                    }
                    else {
                        SDL_SetRenderDrawColor(renderer, 255 * ((float)y / MAP_HEIGHT), 0, 255 * ((float)x / MAP_WIDTH), 255);
                        SDL_RenderFillRect(renderer, &tile);
                    }
                }
            }
        }
    }


    int max_width = TILE_SIZE;
    int max_height = TILE_SIZE * 0.1;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            entity = *get_entity(x, y, height_layer_surface);
            if (entity.type != entity_type_surface_empty) {
                if (entity.max_health > 0 && entity.max_health != entity.health) {
                    int tile_x = entity.x * TILE_SIZE - max_width / 2 + TILE_SIZE / 2;
                    int tile_y = entity.y * TILE_SIZE;
                    SDL_Rect background_rect = { (tile_x - 1), (tile_y - 1), (max_width + 1), (max_height + 1) };
                    SDL_Rect health_rect = { tile_x, tile_y, (max_width * ((float)entity.health / entity.max_health)), max_height };
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_RenderFillRect(renderer, &background_rect);
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    SDL_RenderFillRect(renderer, &health_rect);
                }
            }
        }
    }

    // UI

    SDL_SetRenderTarget(renderer, gui);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    Inventory inventory = main_player->inventory;

    int inventory_width = SCREEN_WIDTH * .6;
    int inventory_height = SCREEN_HEIGHT * .12;


    SDL_Rect inventory_rect = { (SCREEN_WIDTH - inventory_width) / 2,  SCREEN_HEIGHT - inventory_height - 10, inventory_width, inventory_height };
    //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //SDL_RenderFillRect(renderer, &inventory_rect);

    int slot_size = inventory_width / INVENTORY_SIZE;

    int slot_index = 0;
    int padding = 20;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        SDL_Rect slot_rect = { inventory_rect.x + slot_size * i, inventory_rect.y, slot_size, slot_size };
        SDL_RenderCopy(renderer, ui_textures[ui_element_inventory_slot], NULL, &slot_rect);
        if (inventory.content[i].type != item_type_empty) {
            SDL_Rect item_rect = { inventory_rect.x + padding + slot_size * i * slot_index++, inventory_rect.y + padding, slot_size - padding * 2, slot_size - padding * 2 };

            if (item_textures[inventory.content[i].type]) SDL_RenderCopy(renderer, item_textures[inventory.content[i].type], NULL, &item_rect);

            char amount[128];
            sprintf_s(amount, sizeof(amount), "%d", inventory.content[i].amount);

            int digits = floor(log10((double)inventory.content[i].amount)) + 1;
            int digit_size = 24;
            SDL_Rect amount_rect = { slot_rect.x + padding / 2, slot_rect.h + slot_rect.y - padding / 2 - digit_size * 1.5, digit_size * digits , digit_size * 1.5 };

            SDL_Surface* text_surface = TTF_RenderText_Solid(font, amount, (SDL_Color) { 0, 0, 0, 0 });
            text = SDL_CreateTextureFromSurface(renderer, text_surface);
            SDL_RenderCopy(renderer, text, NULL, &amount_rect);
        }
    }

    //

    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderClear(renderer);

    SDL_Rect destination_rect = { -main_camera.x, -main_camera.y, MAP_WIDTH * TILE_SIZE * main_camera.zoom, MAP_HEIGHT * TILE_SIZE * main_camera.zoom };

    SDL_RenderCopy(renderer, screen, NULL, &destination_rect);
    SDL_RenderCopy(renderer, gui, NULL, NULL);
    SDL_RenderPresent(renderer);
}


void init_rendering() {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);  // Exit the program if renderer creation fails
    }

    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);
    if (screen == NULL) {
        printf("Screen texture could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);  // Exit if texture creation fails
    }

    gui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (gui == NULL) {
        printf("GUI texture could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);  // Exit if GUI texture creation fails
    }

    SDL_SetTextureBlendMode(gui, SDL_BLENDMODE_BLEND);
}
