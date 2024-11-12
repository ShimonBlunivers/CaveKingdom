#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "entities/entity.h"
#include "inventory/inventory.h"
#include "audio/audio.h"
#include "world/chunk.h"
#include "world/time.h"
#include "graphics/camera.h"
#include "input/input.h"
#include "networking/networking.h"

int game_status = 1;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Texture* screen = NULL;
SDL_Texture* gui = NULL;
TTF_Font* font = NULL;
SDL_Texture* text = NULL;


typedef enum {
    ui_element_inventory_slot,
    ui_element_death_screen,

    number_of_ui_elements, // DO NOT USE AS UI ELEMENT!
} UIElement;

SDL_Texture* ui_textures[number_of_ui_elements] = { NULL };
SDL_Texture* entity_textures[number_of_entity_types] = { NULL };
SDL_Texture* item_textures[number_of_item_types] = { NULL };

SDL_Texture* hidden_texture = NULL;

void load_textures() {
    // UI textures
    ui_textures[ui_element_inventory_slot] = IMG_LoadTexture(renderer, "./assets/textures/ui/inventory_slot.png");
    ui_textures[ui_element_death_screen] = IMG_LoadTexture(renderer, "./assets/textures/ui/death_screen.png");

    // Entity textures
        // Ground
    entity_textures[entity_type_water] = IMG_LoadTexture(renderer, "./assets/textures/tiles/water.png");
    entity_textures[entity_type_dirt] = IMG_LoadTexture(renderer, "./assets/textures/tiles/dirt.png");

        // Surface
    entity_textures[entity_type_player] = IMG_LoadTexture(renderer, "./assets/textures/tiles/player.png");
    entity_textures[entity_type_enemy] = IMG_LoadTexture(renderer, "./assets/textures/tiles/enemy.png");
    entity_textures[entity_type_zombie] = IMG_LoadTexture(renderer, "./assets/textures/tiles/zombie.png");
    entity_textures[entity_type_stone] = IMG_LoadTexture(renderer, "./assets/textures/tiles/stone.png");
    //entity_textures[entity_type_trunk] = IMG_LoadTexture(renderer, "./assets/textures/tiles/trunk.png");

        // Air
    //entity_textures[entity_type_leaves] = IMG_LoadTexture(renderer, "./assets/textures/tiles/leaves.png");
    //SDL_SetTextureAlphaMod(entity_textures[entity_type_leaves], 128);


    // Item textures
    item_textures[item_type_stone] = IMG_LoadTexture(renderer, "./assets/textures/items/stone.png");
    item_textures[item_type_wood] = IMG_LoadTexture(renderer, "./assets/textures/items/wood.png");
    item_textures[item_type_zombie_meat] = IMG_LoadTexture(renderer, "./assets/textures/items/zombie_meat.png");

    // Special
    hidden_texture = IMG_LoadTexture(renderer, "./assets/textures/tiles/hidden.png");
    //Uint8 r, g, b;
    //SDL_GetTextureColorMod(hidden_texture, &r, &g, &b);
    //SDL_SetTextureColorMod(hidden_texture, r / 2, g / 2, b / 2);
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

    SDL_DestroyTexture(hidden_texture);
}

void load_audio() {

}

void unload_audio() {

}

void init_rendering() {

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE);

    gui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(gui, SDL_BLENDMODE_BLEND);
}


Vector2* vision_edge_positions = NULL;

void load_vision_edge_positions(Vector2** edges) {
    *edges = malloc(sizeof(Vector2) * PLAYER_VIEW_DENSITY);
    if (*edges == NULL) return;

    double angle_increment = (2.0f * M_PI) / PLAYER_VIEW_DENSITY;
    double angle = 0;

    for (int i = 0; i < PLAYER_VIEW_DENSITY; i++) {
        int x = (int)round(cos(angle) * TILE_SIZE * PLAYER_VIEW_DISTANCE);
        int y = (int)round(sin(angle) * TILE_SIZE * PLAYER_VIEW_DISTANCE);
        (*edges)[i] = (Vector2){ x, y };
        angle += angle_increment;
    }
}


void draw_world() {

    SDL_SetRenderTarget(renderer, screen);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderClear(renderer);

    Entity* entity_ptr;

    SDL_Rect tile;

    for (int y = 0, x, layer; y < CHUNK_HEIGHT; y++) {
        for (x = 0; x < CHUNK_WIDTH; x++) {
            tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };
            SDL_RenderCopy(renderer, hidden_texture, NULL, &tile);

            for (layer = 0; layer < number_of_height_layers; layer++) {
                entity_ptr = get_entity(x, y, layer);
                if (entity_ptr != NULL && entity_ptr->visibility != NULL && entity_ptr->visibility->seen && entity_ptr->visibility->last_seen_as != NULL) {
                    SDL_Texture* texture = entity_textures[entity_ptr->visibility->last_seen_as->type];
                    SDL_SetTextureColorMod(texture, 128, 128, 128);
                    SDL_RenderCopyEx(renderer, texture, NULL, &tile, entity_ptr->visibility->last_seen_as->rotation * 90, NULL, false);
                    SDL_SetTextureColorMod(texture, 255, 255, 255);
                }
            }
        }
    }

    int max_width = TILE_SIZE;
    int max_height = (int)(TILE_SIZE * 0.1);
    ///////////////////////////////////
    for (int i = 0, j, layer; i < PLAYER_VIEW_DENSITY; i++) {
        Vector2 end_position = (Vector2){ vision_edge_positions[i].x - PLAYER_VIEW_DISTANCE / 2, vision_edge_positions[i].y - PLAYER_VIEW_DISTANCE / 2 };
        Vector2f direction_vector = vector2f_divide(vector2_to_f(end_position), (Vector2f) { (float)(TILE_SIZE * PLAYER_VIEW_DISTANCE), (float)(TILE_SIZE * PLAYER_VIEW_DISTANCE) });

        //printf("x: %d; y: %d\n", vision_edge_positions[i].x, vision_edge_positions[i].y);
        Vector2 current_position;

        for (j = 0; j < PLAYER_VIEW_DISTANCE; j++) {
            current_position = (Vector2){ (int)(direction_vector.x * j), (int)(direction_vector.y * j) };
            entity_ptr = get_entity(current_position.x + main_player->x, current_position.y + main_player->y, height_layer_surface);
            if (entity_ptr != NULL) {
                for (layer = 0; layer < number_of_height_layers; layer++) {
                    Entity* entity_at_layer = get_entity(entity_ptr->x, entity_ptr->y, layer);
                    if (entity_at_layer != NULL && entity_at_layer->visibility != NULL) {
                        entity_ptr = entity_at_layer;
                        entity_at_layer->visibility->seen = true;
                        entity_at_layer->visibility->last_seen = tick;
                        entity_at_layer->visibility->last_seen_as = entity_at_layer;
                    }
                }
            }
            if (entity_ptr == NULL || !entity_ptr->is_transparent) break;
        }
    }

    
    for (int layer = 0, x, y; layer < number_of_height_layers; layer++) {
        for (y = 0; y < CHUNK_HEIGHT; y++) {
            for (x = 0; x < CHUNK_WIDTH; x++) {
                entity_ptr = get_entity(x, y, layer);
                if (entity_ptr->visibility != NULL && entity_ptr->visibility->last_seen == tick && entity_ptr->visibility->seen && entity_textures[entity_ptr->type] != NULL) {
                    tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };

                    if (entity_ptr->tween != NULL) {
                        Vector2 position = get_current_tween_position(*entity_ptr->tween);
                        tile.x = position.x;
                        tile.y = position.y;

                        if (entity_ptr->tween->finish_tick <= graphic_tick) {
                            delete_tween(entity_ptr->tween);
                            entity_ptr->tween = NULL;
                        }
                    }

                    SDL_RenderCopyEx(renderer, entity_textures[entity_ptr->type], NULL, &tile, entity_ptr->rotation * 90, NULL, false);

                    // Health
                    if (entity_ptr->health->max > 0 && entity_ptr->health->max != entity_ptr->health->value) {
                        int tile_x = tile.x - max_width / 2 + TILE_SIZE / 2;
                        int tile_y = tile.y;
                        SDL_Rect background_rect = { (tile_x - 1), (tile_y - 1), (max_width + 1), (max_height + 1) };
                        SDL_Rect health_rect = { tile_x, tile_y, (int)(max_width * ((float)entity_ptr->health->value / entity_ptr->health->max)), max_height };
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderFillRect(renderer, &background_rect);
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        SDL_RenderFillRect(renderer, &health_rect);
                    }
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


    Inventory inventory;
    if (main_player->inventory != NULL) inventory = *main_player->inventory;
    else inventory = get_empty_inventory();
    

    int inventory_width = (int)(SCREEN_WIDTH * .9);
    int inventory_height = (int)(SCREEN_HEIGHT * .12);


    SDL_Rect inventory_rect = { (SCREEN_WIDTH - inventory_width) / 2,  SCREEN_HEIGHT - inventory_height - 10, inventory_width, inventory_height };

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

            int digits = (int)(floor(log10((double)inventory.content[i].amount))) + 1;
            int digit_size = 24;
            SDL_Rect amount_rect = { (int)(slot_rect.x + padding / 2), (int)(slot_rect.h + slot_rect.y - padding / 2 - digit_size * 1.5), digit_size * digits , (int)(digit_size * 1.5) };

            SDL_Surface* text_surface = TTF_RenderText_Solid(font, amount, (SDL_Color) { 0, 0, 0, 0 });
            text = SDL_CreateTextureFromSurface(renderer, text_surface);
            SDL_RenderCopy(renderer, text, NULL, &amount_rect);
        }
    }

    //

    // Death screen
    if (!main_player_alive) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_RenderFillRect(renderer, NULL);
        SDL_RenderCopy(renderer, ui_textures[ui_element_death_screen], NULL, NULL);
    }
    //

    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderClear(renderer);

    SDL_Rect destination_rect = { -camera.x, -camera.y, (int)(CHUNK_WIDTH * TILE_SIZE * camera.zoom), (int)(CHUNK_HEIGHT * TILE_SIZE * camera.zoom) };


    SDL_RenderCopy(renderer, screen, NULL, &destination_rect);
    SDL_RenderCopy(renderer, gui, NULL, NULL);
    SDL_RenderPresent(renderer);
}


int main(int argc, char* argv[]) {
    srand((int)(time(NULL)));

    reset_grids();
    create_edge_walls();
    generate_world(rand() % 100000);

    //  Testing setup
    {
        Vector2 pos = find_empty_tile();
        force_spawn_entity(new_entity(entity_type_enemy, pos.x, pos.y));
        pos = find_empty_tile();
        force_spawn_entity(new_entity(entity_type_zombie, pos.x, pos.y));
    }

   
    //

    spawn_player();



    // Networking
    setup_server();
    //


    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }
    else if (!main_player) {
        printf("The main player is NULL!\n");
        return 1;
    }
    else if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    else if (!IMG_Init(IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }
    else {
        window = SDL_CreateWindow("CaveKingdom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);

        font = TTF_OpenFont("./assets/fonts/roboto.ttf", 36);

        if (font == NULL)
        {
            printf("Failed to load roboto font! SDL_ttf Error: %s\n", TTF_GetError());
            return 1;
        }
        else if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }
        else {
            init_rendering();
            load_textures();
            load_audio();

            load_vision_edge_positions(&vision_edge_positions);

            Uint32 start_tick = SDL_GetTicks();

            bool quit = false;

            bool player_updated = false;

            Uint32 last_updated = SDL_GetTicks();

            Uint32 update_delay = 250;
            
            while (!quit) {

                update_entities();
                update_server();
                

                while (main_player_alive && !player_updated && SDL_GetTicks() < last_updated + update_delay) {
                    graphic_tick = SDL_GetTicks() - start_tick;
                    quit = process_input();
                    player_updated = update_player();
                    update_camera();
                    draw_world();
                }

                while (SDL_GetTicks() < last_updated + update_delay) {
                    graphic_tick = SDL_GetTicks() - start_tick;
                    update_camera();
                    draw_world();
                }


                player_updated = false;
                last_updated = SDL_GetTicks();
                
                tick++;
            }
        }
    }

    unload_textures();
    unload_audio();

    TTF_CloseFont(font);

    SDL_DestroyWindow(window);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    free_world();
    free(vision_edge_positions);

    close_server();

    return 0;
}