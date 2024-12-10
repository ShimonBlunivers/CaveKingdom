#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "core/game.h"
#include "entities/entity.h"
#include "inventory/inventory.h"
#include "audio/audio.h"
#include "world/chunk.h"
#include "world/time.h"
#include "graphics/camera.h"
#include "input/input.h"
#include "graphics/particles.h"
#include "networking/networking.h"

int game_status = 1;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Texture* screen = NULL;
SDL_Texture* gui = NULL;
TTF_Font* font = NULL;
SDL_Texture* text = NULL;


typedef enum {
    ui_element_selected_inventory_slot,
    ui_element_inventory_slot,
    ui_element_death_screen,
    ui_element_healthbar_outline,

    number_of_ui_elements, // DO NOT USE AS UI ELEMENT!
} UIElement;

SDL_Texture* ui_textures[number_of_ui_elements] = { NULL };
SDL_Texture* entity_textures[number_of_entity_types] = { NULL };
SDL_Texture* item_textures[number_of_item_types] = { NULL };

SDL_Texture* hidden_texture = NULL;
SDL_Texture* shadow_texture = NULL;

void load_textures() {
    // UI textures
    ui_textures[ui_element_selected_inventory_slot] = IMG_LoadTexture(renderer, "./assets/textures/ui/selected_inventory_slot.png");
    ui_textures[ui_element_inventory_slot] = IMG_LoadTexture(renderer, "./assets/textures/ui/inventory_slot.png");
    ui_textures[ui_element_death_screen] = IMG_LoadTexture(renderer, "./assets/textures/ui/death_screen.png");
    ui_textures[ui_element_healthbar_outline] = IMG_LoadTexture(renderer, "./assets/textures/ui/healthbar_outline.png");

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
    hidden_texture = IMG_LoadTexture(renderer, "./assets/textures/special/hidden.png");
    shadow_texture = IMG_LoadTexture(renderer, "./assets/textures/special/shadow.png");
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
    SDL_DestroyTexture(shadow_texture);
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

    {   // Tiles

        Entity* entity_ptr;
        SDL_Rect tile;

        // Checking which tiles are in the player's vision
        for (int i = 0, j, layer; i < PLAYER_VIEW_DENSITY; i++) { 
            Vector2 end_position = (Vector2){ vision_edge_positions[i].x - PLAYER_VIEW_DISTANCE / 2, vision_edge_positions[i].y - PLAYER_VIEW_DISTANCE / 2 };
            Vector2f direction_vector = vector2f_divide(vector2_to_f(end_position), (Vector2f) { (float)(TILE_SIZE * PLAYER_VIEW_DISTANCE), (float)(TILE_SIZE * PLAYER_VIEW_DISTANCE) });

            //printf("x: %d; y: %d\n", vision_edge_positions[i].x, vision_edge_positions[i].y);
            Vector2 current_position;

            for (j = 0; j < PLAYER_VIEW_DISTANCE; j++) {
                current_position = (Vector2){ (int)(direction_vector.x * j), (int)(direction_vector.y * j) };
                entity_ptr = get_entity(current_position.x + main_player->x, current_position.y + main_player->y, height_layer_surface);
                bool transparent = true;
                if (entity_ptr != NULL)
                    for (layer = 0; layer < number_of_height_layers; layer++) {
                        Entity* entity_at_layer = get_entity(entity_ptr->x, entity_ptr->y, layer);
                        if (entity_at_layer != NULL && entity_at_layer->visibility != NULL) {
                            entity_ptr = entity_at_layer;
                            entity_at_layer->visibility->seen = true;
                            entity_at_layer->visibility->last_seen = game_tick;
                            entity_at_layer->visibility->last_seen_as = entity_at_layer;
                            if (!entity_ptr->is_transparent) transparent = false;

                        }
                    }
                if (entity_ptr == NULL || !transparent) break;
            }
        }

        // Rendering seen tiles
        for (int layer = 0, has_tween, y, x; layer < number_of_height_layers; layer++)
        for (has_tween = 0; has_tween <= 1; has_tween++) // Renders tiles with animation on top of others to avoid unexpected layering
        for (y = 0; y < CHUNK_HEIGHT; y++) 
        for (x = 0; x < CHUNK_WIDTH; x++) {
            entity_ptr = get_entity(x, y, layer);
            if (entity_ptr != NULL) {
                if (entity_ptr->tween != NULL) {
                    if (entity_ptr->tween->finish_tick <= (int)graphic_tick) {
                        delete_tween(entity_ptr->tween);
                        entity_ptr->tween = NULL;
                    }
                }
                if (!is_empty_entity_type(entity_ptr->type)) 
                if (has_tween == 0 && entity_ptr->tween == NULL || has_tween == 1 && entity_ptr->tween != NULL)
                if (entity_ptr->visibility != NULL && entity_ptr->visibility->last_seen == game_tick && entity_ptr->visibility->seen &&
                (entity_textures[entity_ptr->type] != NULL || entity_ptr->type == entity_type_dropped_items)) {
                    tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };
                    if (entity_ptr->tween != NULL) {
                        Vector2 position = get_current_tween_position(*entity_ptr->tween);
                        tile.x = position.x;
                        tile.y = position.y;
                    }
                    if (entity_ptr->type == entity_type_dropped_items) { // Rendering items on the ground
                        int item_size = tile.w * 0.25;
                        int padding = 10;

                        int rendered_items = 0;
                        for (int i = 0; i < entity_ptr->inventory->size; i++) {
                            ItemStack item_stack = entity_ptr->inventory->content[i];
					        if (item_stack.type != item_type_empty) {
							    SDL_Rect item_tile = tile;
							    item_tile.w = item_size;
							    item_tile.h = item_size;

							    float relative_x = -tile.w / 2; // Centering
							    float relative_y = -tile.h / 2;

							    relative_x += (rendered_items * item_size) % (tile.w - padding * 2) + padding;
							    relative_y += (rendered_items * item_size) / (tile.w - padding * 2) + padding;

							    relative_x *= cos((float)entity_ptr->rotation / 2);
							    relative_y *= sin((float)entity_ptr->rotation / 2);

							    relative_x += tile.w / 2; // Moving it back
							    relative_y += tile.h / 2;

							    item_tile.x += relative_x;
							    item_tile.y += relative_y;

							    SDL_Rect item_shadow_tile = item_tile;
							    item_shadow_tile.y += 5; // Shadow offset

							    int item_stack_shift = 3;
							    for (int rendered_items_in_stack = 0; rendered_items_in_stack < 4 && rendered_items_in_stack < item_stack.amount; rendered_items_in_stack++) {
								    item_tile.x += item_stack_shift * rendered_items_in_stack;
								    item_shadow_tile.x += item_stack_shift * rendered_items_in_stack;
								    SDL_RenderCopyEx(renderer, shadow_texture, NULL, &item_shadow_tile, (rendered_items_in_stack + entity_ptr->rotation) * 90, NULL, false);

								    SDL_RenderCopyEx(renderer, item_textures[item_stack.type], NULL, &item_tile,  (rendered_items_in_stack + entity_ptr->rotation)  * 90, NULL, false);
							    }
                           
							    rendered_items++;
                            }
                        }
                    } 
                    else SDL_RenderCopyEx(renderer, entity_textures[entity_ptr->type], NULL, &tile, entity_ptr->rotation * 90, NULL, false);
                }
            }
        }

        // Rendering the every unseen as darker
        for (int y = 0, x, layer; y < CHUNK_HEIGHT; y++)
        for (x = 0; x < CHUNK_WIDTH; x++) {
            tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };
            for (layer = 0; layer < number_of_height_layers; layer++) {
                entity_ptr = get_entity(x, y, layer);
                if (entity_ptr != NULL && !is_empty_entity_type(entity_ptr->type) && entity_ptr->visibility != NULL && entity_ptr->visibility->seen && entity_ptr->visibility->last_seen != game_tick && entity_ptr->visibility->last_seen_as != NULL) {
                    SDL_Texture* texture = entity_textures[entity_ptr->visibility->last_seen_as->type];
                    SDL_SetTextureColorMod(texture, 128, 128, 128);
                    SDL_RenderCopyEx(renderer, texture, NULL, &tile, entity_ptr->visibility->last_seen_as->rotation * 90, NULL, false);
                    SDL_SetTextureColorMod(texture, 255, 255, 255);
                }
            }
        }

        for (int y = 0, x, layer; y < CHUNK_HEIGHT; y++) // Rendering every hidden
        for (x = 0; x < CHUNK_WIDTH; x++) {
            bool hidden = true;
            for (layer = 0; layer < number_of_height_layers; layer++) {

                entity_ptr = get_entity(x, y, layer);

                if (entity_ptr != NULL && entity_ptr->visibility != NULL && entity_ptr->visibility->seen && entity_ptr->visibility->last_seen_as != NULL) {
                    hidden = false;
                }
            }
            if (hidden) {
                float hidden_tile_offset = TILE_SIZE * 0.3;
                tile = (SDL_Rect){ TILE_SIZE * x - (int)round(hidden_tile_offset), TILE_SIZE * y - (int)round(hidden_tile_offset), TILE_SIZE + (int)round(hidden_tile_offset * 2), TILE_SIZE + (int)round(hidden_tile_offset * 2) };
                SDL_RenderCopy(renderer, hidden_texture, NULL, &tile);
            }
        }
    }

    {   // Particles
        SDL_Rect particle_rect;
        ParticleListItem* item = PARTICLE_MANAGER.first_particle;
        while (item != NULL) {
            particle_rect = (SDL_Rect){ (int)round(item->particle.x), (int)round(item->particle.y), item->particle.size, item->particle.size };
            SDL_SetRenderDrawColor(renderer, item->particle.color.r, item->particle.color.g, item->particle.color.b, item->particle.color.a);
            SDL_RenderFillRect(renderer, &particle_rect);
            item = item->next_list_item;
        }
    }
    
    {   // UI

        {   // Healthbars
            Entity* entity_ptr;
            SDL_Rect tile;
            int max_healthbar_width = TILE_SIZE;
            int max_healthbar_height = TILE_SIZE / 5;
            for (int y = 0, x, layer; y < CHUNK_HEIGHT; y++)
            for (x = 0; x < CHUNK_WIDTH; x++) 
            for (layer = 0; layer < number_of_height_layers; layer++) {
                entity_ptr = get_entity(x, y, layer);
                if (entity_ptr != NULL && thermal_vision) { // !!!!!!!!!
                    tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };
                    SDL_SetTextureColorMod(entity_textures[entity_ptr->type], (int)(255 * (entity_ptr->thermal.temperature - 250) / entity_ptr->thermal.max_temperature) % 255, 0, 0);
                    SDL_RenderCopyEx(renderer, entity_textures[entity_ptr->type], NULL, &tile, entity_ptr->rotation * 90, NULL, false);
                    SDL_SetTextureColorMod(entity_textures[entity_ptr->type], 255, 255, 255);
                }// !!!!!!!!!!!!
                if (entity_ptr->visibility != NULL && !is_empty_entity_type(entity_ptr->type) && entity_ptr->visibility->last_seen == game_tick && entity_ptr->visibility->seen && entity_textures[entity_ptr->type] != NULL) {
                    if (entity_ptr->health->max > 0 && entity_ptr->health->max != entity_ptr->health->current) {
                        tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };
                        if (entity_ptr->tween != NULL) {
                            Vector2 position = get_current_tween_position(*entity_ptr->tween);
                            tile.x = position.x;
                            tile.y = position.y;
                        }
                        int tile_x = tile.x - max_healthbar_width / 2 + TILE_SIZE / 2;
                        int tile_y = tile.y;

                        Vector2 padding = { 2, 3 };
                        SDL_Rect outline_rect = { tile_x, tile_y, max_healthbar_width, max_healthbar_height };
                        SDL_Rect background_rect = { outline_rect.x + padding.x, outline_rect.y + padding.y, outline_rect.w - padding.x * 2, outline_rect.h - padding.y * 2 };
                        SDL_Rect health_rect = { background_rect.x, background_rect.y, (int)(background_rect.w * ((float)entity_ptr->health->current / entity_ptr->health->max)), background_rect.h };

                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderFillRect(renderer, &background_rect);
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        SDL_RenderFillRect(renderer, &health_rect);
                        SDL_RenderCopy(renderer, ui_textures[ui_element_healthbar_outline], NULL, &outline_rect);
                    }
                }
            }
        }

        SDL_SetRenderTarget(renderer, gui);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);


        Inventory* inventory = main_player->inventory;

        int inventory_width = (int)(SCREEN_WIDTH * .9);
        int inventory_height = (int)(SCREEN_HEIGHT * .12);

        SDL_Rect inventory_rect = { (SCREEN_WIDTH - inventory_width) / 2,  SCREEN_HEIGHT - inventory_height - 10, inventory_width, inventory_height };

        int slot_size = inventory_width / INVENTORY_SIZE;

        int slot_index = 0;
        int padding = 20;

        int text_padding = 15;

        float text_scale = 0.65f;

        float outline_offset = 2.0;

        for (int i = 0; i < INVENTORY_HOTBAR_SLOTS; i++) {
            SDL_Rect slot_rect = { inventory_rect.x + slot_size * i, inventory_rect.y, slot_size, slot_size };
            SDL_RenderCopy(renderer, ui_textures[ui_element_inventory_slot], NULL, &slot_rect);

            if (inventory == NULL || i >= inventory->size) continue;

            if (inventory->selected_slot == i) SDL_RenderCopy(renderer, ui_textures[ui_element_selected_inventory_slot], NULL, &slot_rect);
            if (inventory->content[i].type != item_type_empty) {
                SDL_Rect item_rect = { inventory_rect.x + padding + slot_size * i * slot_index++, inventory_rect.y + padding, slot_size - padding * 2, slot_size - padding * 2 };

                if (item_textures[inventory->content[i].type]) SDL_RenderCopy(renderer, item_textures[inventory->content[i].type], NULL, &item_rect);

                char amount[128];
                sprintf_s(amount, sizeof(amount), "%d", inventory->content[i].amount);

                int text_width, text_height;

                TTF_SizeUTF8(font, amount, &text_width, &text_height);

                SDL_Rect amount_rect = { 
                    .x = (int)round(slot_rect.x + text_padding),
                    .y = (int)round(slot_rect.y + slot_rect.h - text_padding / 2 - text_height * text_scale),
                    .w = (int)round(text_width * text_scale),
                    .h = (int)round(text_height * text_scale),
                };

                SDL_Color text_color =  { 255, 255, 255, 255 };
                SDL_Color text_outline_color = { 0, 0, 0, 255 };

                SDL_Surface* text_outline_surface = TTF_RenderText_Blended(font, amount, text_outline_color);

                SDL_Surface* text_surface = TTF_RenderText_Blended(font, amount, text_color);
                
                SDL_Texture* outline_texture = SDL_CreateTextureFromSurface(renderer, text_outline_surface);

                SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

                SDL_FreeSurface(text_surface);
                SDL_FreeSurface(text_outline_surface);
                    
                SDL_Rect outline_rect = {
                    .w = amount_rect.w,
                    .h = amount_rect.h,
                };

                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue; // Skip the center position
                        outline_rect.x = (int)round(amount_rect.x + dx * outline_offset);
                        outline_rect.y = (int)round(amount_rect.y + dy * outline_offset);
                        SDL_RenderCopy(renderer, outline_texture, NULL, &outline_rect);
                    }
                }

                SDL_RenderCopy(renderer, text_texture, NULL, &amount_rect);

                SDL_DestroyTexture(outline_texture);
                SDL_DestroyTexture(text_texture);
            }
        }
    }

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

    init_chunk_manager();


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
        fprintf(stderr, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }
    else if (!main_player) {
        fprintf(stderr, "The main player is NULL!\n");
        return 1;
    }
    else if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    else if (!IMG_Init(IMG_INIT_PNG)) {
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }
    else {
        //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 16);

        window = SDL_CreateWindow("CaveKingdom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);

        font = TTF_OpenFont("./assets/fonts/roboto.ttf", 32);

        if (font == NULL)
        {
            fprintf(stderr, "Failed to load roboto font! SDL_ttf Error: %s\n", TTF_GetError());
            return 1;
        }
        else if (window == NULL) {
            fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }
        else {
            init_rendering();
            init_input();
            load_textures();
            load_audio();

            load_vision_edge_positions(&vision_edge_positions);

            bool quit = false;

            bool player_updated;

            graphic_tick = SDL_GetTicks();

            previous_counter = SDL_GetPerformanceCounter();

            update_time();
            Uint32 update_delay = 250; // 250
            //Uint32 ummovable_delay = 100; // To prevent player moving twice when he presses his command at the end of the cycle.
            while (!quit) {
                last_updated_tick = SDL_GetTicks();

                player_updated = false;

                update_entities();
                update_server();
                
                while (graphic_tick < last_updated_tick + update_delay) {
                    quit = process_input();
                    update_player_inventory();
                    
                    if (!player_updated && main_player_alive/* && graphic_tick > last_updated_tick + ummovable_delay*/) {
                        player_updated = update_player();
                    }


                    update_camera();
                    update_particles();
                    draw_world();

                    update_time();
                }

                game_tick++;
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