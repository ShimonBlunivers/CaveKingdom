#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "entity.h"
#include "inventory.h"
#include "chunk.h"
#include "camera.h"
#include "input.h"

#include "networking.h"

int game_status = 1;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Texture* screen = NULL;
SDL_Texture* gui = NULL;
TTF_Font* font = NULL;
SDL_Texture* text = NULL;


typedef enum {
    ui_element_inventory_slot,

    number_of_ui_elements, // DO NOT USE AS UI ELEMENT!
} UIElement;

SDL_Texture* ui_textures[number_of_ui_elements] = { 0 };
SDL_Texture* entity_textures[number_of_entity_types] = { 0 };
SDL_Texture* item_textures[number_of_item_types] = { 0 };

SDL_Texture* hidden_texture = 0;

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
    //entity_textures[entity_type_trunk] = IMG_LoadTexture(renderer, "./assets/textures/tiles/trunk.png");

        // Air
    //entity_textures[entity_type_leaves] = IMG_LoadTexture(renderer, "./assets/textures/tiles/leaves.png");
    //SDL_SetTextureAlphaMod(entity_textures[entity_type_leaves], 128);

    // Item textures
    item_textures[item_type_stone] = IMG_LoadTexture(renderer, "./assets/textures/items/stone.png");
    item_textures[item_type_wood] = IMG_LoadTexture(renderer, "./assets/textures/items/wood.png");
    item_textures[item_type_zombie_meat] = IMG_LoadTexture(renderer, "./assets/textures/items/zombie_meat.png");

    // Special
    hidden_texture = IMG_LoadTexture(renderer, "./assets/textures/tiles/stone.png");
    Uint8 r, g, b;
    SDL_GetTextureColorMod(hidden_texture, &r, &g, &b);
    SDL_SetTextureColorMod(hidden_texture, r / 2, g / 2, b / 2);
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

void init_rendering() {

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);

    gui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(gui, SDL_BLENDMODE_BLEND);
}

//Entity* visible_tiles[PLAYER_VISION * PLAYER_VISION] = { NULL };
void draw_world() {
    //memset(visible_tiles, NULL, PLAYER_VISION * PLAYER_VISION);


    Vector2 vision_edge_positions[PLAYER_VISION * 4 - 4];

    {
        int index = 0;
        // Top edge (left to right)
        for (int x = 0; x < PLAYER_VISION; x++) {
            vision_edge_positions[index++] = (Vector2){ x, 0 };
        }

        // Right edge (top to bottom, excluding top corner)
        for (int y = 1; y < PLAYER_VISION; y++) {
            vision_edge_positions[index++] = (Vector2){ PLAYER_VISION - 1, y };
        }

        // Bottom edge (right to left, excluding bottom-right corner)
        for (int x = PLAYER_VISION - 2; x >= 0; x--) {
            vision_edge_positions[index++] = (Vector2){ x, PLAYER_VISION - 1 };
        }

        // Left edge (bottom to top, excluding top-left and bottom-left corners)
        for (int y = PLAYER_VISION - 2; y > 0; y--) {
            vision_edge_positions[index++] = (Vector2){ 0, y };
        }
    }


    SDL_SetRenderTarget(renderer, screen);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderClear(renderer);

    Entity entity;
    Entity* entity_ptr;

    SDL_Rect tile;
    SDL_Color tile_color;


    //for (int y = 0; y < PLAYER_VISION; y++) {
    //    for (int x = 0; x < PLAYER_VISION; x++) {
    //        entity_ptr = get_entity(x + main_player->x - PLAYER_VISION / 2, y + main_player->y - PLAYER_VISION / 2, height_layer_ground);
    //        if (entity_ptr != NULL) visible_tiles[y * PLAYER_VISION + x] = entity_ptr;
    //    }
    //}


    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            for (int layer = 0; layer < number_of_height_layers; layer++) {
                entity = *get_entity(x, y, layer);
                tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, hidden_texture, NULL, &tile);
            }
        }
    }
    //            //if (!is_empty_entity_type(entity.type)) {
    //                //if (entity_textures[entity.type] != NULL) {
    //                //    SDL_RenderCopy(renderer, entity_textures[entity.type], NULL, &tile);
    //                //}
    //                //else {
    //                //    SDL_SetRenderDrawColor(renderer, 255 * ((float)y / MAP_HEIGHT), 0, 255 * ((float)x / MAP_WIDTH), 255);
    //                //    SDL_RenderFillRect(renderer, &tile);
    //                //}
    //            //}
    //        }
    //    }
    //}

    //for (int y = 0; y < PLAYER_VISION; y++) {
    //    for (int x = 0; x < PLAYER_VISION; x++) {
    //        entity_ptr = visible_tiles[y * PLAYER_VISION + x];
    //        if (entity_ptr != NULL) {
    //            for (int layer = 0; layer < number_of_height_layers; layer++) {
    //                Entity* entity_at_layer = get_entity(entity_ptr->x, entity_ptr->y, layer);
    //                if (entity_at_layer != NULL) {
    //                    entity = *entity_at_layer;
    //                    if (entity_textures[entity.type] != NULL) {
    //                        tile = (SDL_Rect){ TILE_SIZE * entity.x, TILE_SIZE * entity.y, TILE_SIZE, TILE_SIZE };
    //                        SDL_RenderCopy(renderer, entity_textures[entity.type], NULL, &tile);
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}


    int max_width = TILE_SIZE;
    int max_height = TILE_SIZE * 0.1;
    ///////////////////////////////////
    for (int i = 0; i < PLAYER_VISION * 4 - 4; i++) {
        int j = 1;
        Vector2 end_position = (Vector2){ vision_edge_positions[i].x - PLAYER_VISION / 2, vision_edge_positions[i].y - PLAYER_VISION / 2 };
        Vector2f direction_vector = vector2f_normalize((Vector2f) { (float)end_position.x, (float)end_position.y });
        Vector2 current_position;

        do {
            current_position = (Vector2){ (int)(direction_vector.x * j), (int)(direction_vector.y * j) };
            entity_ptr = get_entity(current_position.x + main_player->x, current_position.y + main_player->y, height_layer_surface);
            if (entity_ptr != NULL) {
                for (int layer = 0; layer < number_of_height_layers; layer++) {
                    Entity* entity_at_layer = get_entity(entity_ptr->x, entity_ptr->y, layer);
                    if (entity_at_layer != NULL) {
                        entity = *entity_at_layer;
                        if (entity_textures[entity.type] != NULL) {
                            tile = (SDL_Rect){ TILE_SIZE * entity.x, TILE_SIZE * entity.y, TILE_SIZE, TILE_SIZE };
                            SDL_RenderCopy(renderer, entity_textures[entity.type], NULL, &tile);
                            if (entity.health.max > 0 && entity.health.max != entity.health.value) {
                                int tile_x = entity.x * TILE_SIZE - max_width / 2 + TILE_SIZE / 2;
                                int tile_y = entity.y * TILE_SIZE;
                                SDL_Rect background_rect = { (tile_x - 1), (tile_y - 1), (max_width + 1), (max_height + 1) };
                                SDL_Rect health_rect = { tile_x, tile_y, (max_width * ((float)entity.health.value / entity.health.max)), max_height };
                                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                                SDL_RenderFillRect(renderer, &background_rect);
                                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                                SDL_RenderFillRect(renderer, &health_rect);
                            }
                        }

                    }
                }
            }
            j++;
        } while (
            entity_ptr != NULL &&
            entity_ptr->is_transparent &&
            !vector2_equals(current_position, end_position) &&
            abs(current_position.x) <= abs(end_position.x) &&
            abs(current_position.y) <= abs(end_position.y)
            );
    }


    //for (int y = 0; y < MAP_HEIGHT; y++) {
    //    for (int x = 0; x < MAP_WIDTH; x++) {
    //        entity = *get_entity(x, y, height_layer_surface);
    //        if (entity.type != entity_type_surface_empty) {
    //            if (entity.health.max > 0 && entity.health.max != entity.health.value) {
    //                int tile_x = entity.x * TILE_SIZE - max_width / 2 + TILE_SIZE / 2;
    //                int tile_y = entity.y * TILE_SIZE;
    //                SDL_Rect background_rect = { (tile_x - 1), (tile_y - 1), (max_width + 1), (max_height + 1) };
    //                SDL_Rect health_rect = { tile_x, tile_y, (max_width * ((float)entity.health.value / entity.health.max)), max_height };
    //                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //                SDL_RenderFillRect(renderer, &background_rect);
    //                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    //                SDL_RenderFillRect(renderer, &health_rect);
    //            }
    //        }
    //    }
    //}

    // UI

    SDL_SetRenderTarget(renderer, gui);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    Inventory inventory = main_player->inventory;

    int inventory_width = SCREEN_WIDTH * .9;
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

    SDL_Rect destination_rect = { -camera.x, -camera.y, MAP_WIDTH * TILE_SIZE * camera.zoom, MAP_HEIGHT * TILE_SIZE * camera.zoom };

    SDL_RenderCopy(renderer, screen, NULL, &destination_rect);
    SDL_RenderCopy(renderer, gui, NULL, NULL);
    SDL_RenderPresent(renderer);
}


int main(void) {
    srand(time(NULL));

    reset_grids();
    create_edge_walls();

    //  Testing setup
    
    spawn_entity(new_entity(entity_type_player, 11, 11));

    //spawn_entity(new_entity(entity_type_wall, 3, 4));
    spawn_entity(new_entity(entity_type_enemy, 2, 4));
    spawn_entity(new_entity(entity_type_zombie, 10, 4));

    for (int i = 0; i < 10; i++) {
        spawn_entity(new_entity(entity_type_stone, 2 + i, 6));
        spawn_entity(new_entity(entity_type_stone, 3 + i, 7));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 8));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 9));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 10));
    }


    //spawn_entity(new_entity(entity_type_trunk, 6, 13));
    //spawn_entity(new_entity(entity_type_trunk, 12, 13));

    //
    
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
    else if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    else if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
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

            bool quit = false;

            SDL_Event event;
            while (!quit) {
                
                quit = process_input();

                update_entities();

                update_server();

                update_camera();
                draw_world();

                SDL_Delay(100);
            }
        }
    }

    unload_textures();

    TTF_CloseFont(font);

    SDL_DestroyWindow(window);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    close_server();

    return 0;
}