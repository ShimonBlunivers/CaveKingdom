#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define MAP_WIDTH 36
#define MAP_HEIGHT 36
#define TILE_SIZE 60

#define INVENTORY_SIZE 8

#pragma region Window

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int game_status = 1;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Texture* screen = NULL;
SDL_Texture* gui = NULL;
TTF_Font* font = NULL;
SDL_Texture* text;

#pragma endregion

#pragma region Items

typedef enum {
    item_type_empty,

    item_type_stone,
    item_type_zombie_meat,


    number_of_item_types, // DO NOT USE AS ITEM TYPE!
} ItemType;

typedef struct {
    ItemType type;
    int amount;
} ItemStack;

typedef struct {
    ItemStack content[INVENTORY_SIZE];
} Inventory;

Inventory new_inventory(ItemStack content[INVENTORY_SIZE]) {
    Inventory inventory;

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (content != NULL && content[i].type != item_type_empty) inventory.content[i] = content[i];
        else inventory.content[i] = (ItemStack) {item_type_empty, -1};
    }

    return inventory;
}

void collect_inventory(Inventory* from, Inventory* to) {
    for (int x = 0; x < INVENTORY_SIZE; x++) {
        if (from->content[x].type == item_type_empty) continue;
        for (int y = 0; y < INVENTORY_SIZE; y++) {
            if (to->content[y].type == from->content[x].type) {
                to->content[y].amount += from->content[x].amount;
                from->content[x] = (ItemStack){ item_type_empty, -1 };
                continue;
            }
            if (to->content[y].type == item_type_empty) {
                to->content[y] = from->content[x];
                from->content[x] = (ItemStack){ item_type_empty, -1 };
                continue;
            }
        }
    }
}

void print_inventory(Inventory* inventory) {
    printf("------------\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory->content[i].type != item_type_empty) {
            printf("%d : %d\n", inventory->content[i].type, inventory->content[i].amount);
        }
    }
}

#pragma endregion

#pragma region Entities

typedef enum {
    height_layer_ground,
    height_layer_surface,
    height_layer_air,

    number_of_height_layers, // DO NOT USE AS HEIGHT LAYER !
} HeightLayer;

typedef enum {
    // Empty types for every height layer
    entity_type_ground_empty,
    entity_type_surface_empty,
    entity_type_air_empty,

    // Ground types
    entity_type_water,
    entity_type_dirt,

    // Surface types
    entity_type_player,
    entity_type_enemy,
    entity_type_zombie,
    entity_type_wall,
    entity_type_stone,

    number_of_entity_types, // DO NOT USE AS ENTITY TYPE !
} EntityType;

EntityType empty_entity_types[number_of_height_layers] = { 
    entity_type_ground_empty, 
    entity_type_surface_empty,
    entity_type_air_empty,
}; // List of empty types for every layer, ascending.

bool is_empty_entity_type(EntityType entity_type) {
    for (int i = 0; i < number_of_height_layers; i++) if (entity_type == empty_entity_types[i]) return true;
    return false;
}

typedef struct {
    EntityType type;
    HeightLayer height_layer;
    int x;
    int y;
    int max_health;
    int health;
    bool is_obstacle;
    Inventory inventory;
} Entity;

Entity new_entity(EntityType type, int x, int y) {
    HeightLayer height_layer;

    Entity new_entity = { 0 };
    ItemStack loot[INVENTORY_SIZE] = {0};

    height_layer = height_layer_ground;
    switch (type) {
        case entity_type_water:
            new_entity = (Entity) { type, height_layer, x, y, -1, NULL, true, new_inventory(loot) };
            break;

        case entity_type_dirt:
            new_entity = (Entity) { type, height_layer, x, y, -1, NULL, false, new_inventory(loot) };
            break;

        case entity_type_ground_empty:
            new_entity = (Entity) { type, height_layer, x, y, -1, NULL, false, new_inventory(loot) };
            break;
    }

    height_layer = height_layer_surface;
    switch (type) {
        case entity_type_player:
            new_entity = (Entity){ type, height_layer, x, y, 10, NULL, true, new_inventory(loot) };
            break;

        case entity_type_enemy:
            new_entity = (Entity){ type, height_layer, x, y, 10, NULL, true, new_inventory(loot)};
            break;

        case entity_type_zombie:
            loot[0] = (ItemStack){ item_type_zombie_meat, 2 };
            new_entity = (Entity){ type, height_layer, x, y, 10, NULL, true, new_inventory(loot) };
            break;

        case entity_type_wall:
            new_entity = (Entity){ type, height_layer, x, y, -1, NULL, true, new_inventory(loot) };
            break;

        case entity_type_stone:
            loot[0] = (ItemStack){ item_type_stone, 3 };
            new_entity = (Entity){ type, height_layer, x, y, 5, NULL, true, new_inventory(loot) };
            break;

        case entity_type_surface_empty:
            new_entity = (Entity){ type, height_layer, x, y, -1, NULL, false, new_inventory(loot) };
            break;    
    }
    if (new_entity.max_health == 0) return new_entity;

    new_entity.health = new_entity.max_health;
    return new_entity;
}

static Entity entity_list[MAP_WIDTH * MAP_HEIGHT * number_of_height_layers];
static Entity* entity_position_grid[MAP_WIDTH * MAP_HEIGHT][number_of_height_layers]; // For quicker access through coordinates

Entity* main_player = NULL;

void destroy_entity(Entity* entity) {
    Entity empty_entity = new_entity(empty_entity_types[entity->height_layer], entity->x, entity->y);

    *entity_position_grid[entity->y * MAP_WIDTH + entity->x][empty_entity.height_layer] = empty_entity;
}

Entity* get_entity(int x, int y, HeightLayer layer) {
    if ( x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return NULL;
    return entity_position_grid[y * MAP_WIDTH + x][layer];
}

bool set_entity(int x, int y, Entity* enity) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
    entity_position_grid[y * MAP_WIDTH + x][enity->height_layer] = enity;
    return true;
}

bool spawn_entity(Entity entity) {
    if (entity.x < 0 || entity.y < 0 || entity.x >= MAP_WIDTH || entity.y >= MAP_HEIGHT) return false;
    Entity* old_entity = get_entity(entity.x, entity.y, entity.height_layer);

    if (!is_empty_entity_type(old_entity->type)) return false;
    *old_entity = entity;
    if (entity.type == entity_type_player) {
        main_player = old_entity;
    }
    return true;
}

bool force_spawn_entity(Entity entity) {
    if (entity.x < 0 || entity.y < 0 || entity.x >= MAP_WIDTH || entity.y >= MAP_HEIGHT) return false;
    Entity* old_entity = get_entity(entity.x, entity.y, entity.height_layer);

    *old_entity = entity;
    if (entity.type == entity_type_player) {
        main_player = old_entity;
    }
    return true;
}

void hit_entity(Entity* entity, int damage) {
    if (entity->max_health < 0) return;
    entity->health -= damage;
    if (entity->health <= 0) { 
        collect_inventory(&entity->inventory, &main_player->inventory);
        destroy_entity(entity); 
    }
}

bool switch_entities(int x1, int y1, int x2, int y2, HeightLayer layer) {
    if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 >= MAP_WIDTH || x2 >= MAP_WIDTH || y1 >= MAP_HEIGHT || y2 >= MAP_HEIGHT) return false;

    Entity* entity1 = get_entity(x1, y1, layer);
    Entity* entity2 = get_entity(x2, y2, layer);

    if (entity1->type == entity_type_player && entity2->max_health > 0) {
        hit_entity(entity2, 1);
        return false;
    }
    if (entity2->type == entity_type_player && entity1->max_health > 0) {
        hit_entity(entity1, 1);
        return false;
    }

    for (int i = 0; i < number_of_height_layers; i++) {
        Entity* entity = get_entity(x2, y2, i);
        if (entity->is_obstacle) return false;
    }

    entity2->x = x1;
    entity2->y = y1;

    entity1->x = x2;
    entity1->y = y2;

    Entity* temp = entity1;
    set_entity(x1, y1, entity2);
    set_entity(x2, y2, temp);

    return true;
}

#pragma endregion

#pragma region Textures

typedef enum {
    ui_element_inventory_slot,

    number_of_ui_elements, // DO NOT USE AS UI ELEMENT!
} UIElement;


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

#pragma endregion

#pragma region Camera

typedef struct {
    int x;
    int y;
    float zoom;
    float min_zoom;
    float max_zoom;
} Camera;


Camera main_camera = { 0, 0, 1. , .1, 3. };

void update_camera() {
    main_camera.x = TILE_SIZE * main_camera.zoom * (0.5 + main_player->x) - 0.5 * SCREEN_WIDTH;
    main_camera.y = TILE_SIZE * main_camera.zoom * (0.5 + main_player->y) - 0.5 * SCREEN_HEIGHT;
}

#pragma endregion

#pragma region Grid

void reset_grids() {
    for (int layer = 0; layer < number_of_height_layers; layer++) {
        for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
            if (layer == height_layer_ground) entity_list[i + MAP_WIDTH * MAP_HEIGHT * layer] = new_entity(entity_type_dirt, i % MAP_WIDTH, i / MAP_WIDTH);
            if (layer == height_layer_surface) entity_list[i + MAP_WIDTH * MAP_HEIGHT * layer] = new_entity(entity_type_surface_empty, i % MAP_WIDTH, i / MAP_WIDTH);
            
            entity_position_grid[i][layer] = &entity_list[i + MAP_WIDTH * MAP_HEIGHT * layer];
        }
    }
}

void create_edge_walls() {
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) {
                force_spawn_entity(new_entity(entity_type_water, x, y));
            }
}

void update_world(int player_movement_x, int player_movement_y) {
    if (player_movement_x == 0 && player_movement_y == 0) return;
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT * number_of_height_layers; i++)
    {
        Entity* entity = &entity_list[i];
        switch (entity->type) {
            case entity_type_surface_empty:
                break;

            case entity_type_wall:
                break;

            case entity_type_player:
                if (player_movement_x != 0)
                    switch_entities(entity->x, entity->y, entity->x + player_movement_x, entity->y, main_player->height_layer);
                if (player_movement_y != 0)
                    switch_entities(entity->x, entity->y, entity->x, entity->y + player_movement_y, main_player->height_layer);
                break;

            case entity_type_enemy:
                break;
        }
    }
}

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
                    if (entity_textures[entity.type]) {
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
        SDL_Rect slot_rect = { inventory_rect.x + slot_size * i, inventory_rect.y, slot_size, slot_size};
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


#pragma endregion

#pragma region Game

void init_rendering() {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);
    
    gui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(gui, SDL_BLENDMODE_BLEND);
}

int main(void) {
    
    reset_grids();
    create_edge_walls();

    //  Testing setup
    
    spawn_entity(new_entity(entity_type_player, 11, 11));


    //spawn_entity(new_entity(entity_type_wall, 3, 4));
    spawn_entity(new_entity(entity_type_enemy, 2, 4));
    spawn_entity(new_entity(entity_type_zombie, 3, 4));



    for (int i = 0; i < 10; i++) {
        spawn_entity(new_entity(entity_type_stone, 2 + i, 6));
        spawn_entity(new_entity(entity_type_stone, 3 + i, 7));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 8));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 9));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 10));
    }


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

            SDL_Event event;
            
            int quit = 0;

            int player_movement_x, player_movement_y;
            int w_key_pressed = 0;
            int s_key_pressed = 0;
            int a_key_pressed = 0;
            int d_key_pressed = 0;

            while (quit == 0) { 
                player_movement_x = 0;
                player_movement_y = 0;

                while (SDL_PollEvent(&event)) { 
                    if (event.type == SDL_QUIT) quit = 1; 
                    else if (event.type == SDL_KEYDOWN) {
                        if (SDLK_w == event.key.keysym.sym) w_key_pressed = 1;
                        else if (SDLK_s == event.key.keysym.sym) s_key_pressed = 1;
                        else if (SDLK_a == event.key.keysym.sym) a_key_pressed = 1;
                        else if (SDLK_d == event.key.keysym.sym) d_key_pressed = 1;
                        else if (SDLK_e == event.key.keysym.sym) main_camera.zoom = SDL_clamp(main_camera.zoom + .1, main_camera.min_zoom, main_camera.max_zoom);
                        else if (SDLK_q == event.key.keysym.sym) main_camera.zoom = SDL_clamp(main_camera.zoom - .1, main_camera.min_zoom, main_camera.max_zoom);
                        else if (SDLK_LEFT == event.key.keysym.sym) main_camera.x -= 10;
                        else if (SDLK_RIGHT == event.key.keysym.sym) main_camera.x += 10;
                        else if (SDLK_UP == event.key.keysym.sym) main_camera.y -= 10;
                        else if (SDLK_DOWN == event.key.keysym.sym) main_camera.y += 10;

                        //else if (SDLK_r == event.key.keysym.sym) print_inventory(&main_player->inventory);

                        
                    }
                    else if (event.type == SDL_KEYUP) {
                        if (SDLK_w == event.key.keysym.sym) w_key_pressed = 0;
                        else if (SDLK_s == event.key.keysym.sym) s_key_pressed = 0;
                        else if (SDLK_a == event.key.keysym.sym) a_key_pressed = 0;
                        else if (SDLK_d == event.key.keysym.sym) d_key_pressed = 0;
                    }
                } 
                if (w_key_pressed) player_movement_y--;
                if (s_key_pressed) player_movement_y++;
                if (a_key_pressed) player_movement_x--;
                if (d_key_pressed) player_movement_x++;

                update_world(player_movement_x, player_movement_y);
                update_camera();
                draw_world();

                SDL_Delay(100);
            }
        }
    }


    for (int i = 0; i < number_of_ui_elements; i++) {
        SDL_DestroyTexture(ui_textures[i]);
    }
    for (int i = 0; i < number_of_entity_types; i++) {
        SDL_DestroyTexture(entity_textures[i]);
    }
    for (int i = 0; i < number_of_item_types; i++) {
        SDL_DestroyTexture(item_textures[i]);
    }

    TTF_CloseFont(font);

    SDL_DestroyWindow(window);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}

#pragma endregion