#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL_image.h>
#include <SDL.h>


#define MAP_WIDTH 36
#define MAP_HEIGHT 36
#define TILE_SIZE 20



//  WINDOW

const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 720;

int game_status = 1;

SDL_Window* window = NULL;
SDL_Texture* screen = NULL;
SDL_Renderer* renderer = NULL;

//  ENTITIES

typedef enum {
    entity_type_player,
    entity_type_wall,
    entity_type_enemy,
    entity_type_stone,

    entity_type_empty,
} EntityType;

typedef struct {
    EntityType entity_type;
    int x;
    int y;
    int max_health;
    int health;
    SDL_Color color;
} Entity;

static Entity entities[MAP_WIDTH * MAP_HEIGHT];
static Entity* grid[MAP_WIDTH * MAP_HEIGHT];

Entity* main_player = NULL;

Entity new_entity(EntityType type, int x, int y) {

    switch (type) {
    case entity_type_player:
        return (Entity) { entity_type_player, x, y, 10, 10, (SDL_Color) { 0, 0, 255, 255 } };

    case entity_type_wall:
        return (Entity) { entity_type_wall, x, y, -1, -1, (SDL_Color) { 32, 32, 32, 255 } };

    case entity_type_stone:
        return (Entity) { entity_type_stone, x, y, 5, 5, (SDL_Color) { 64, 64, 64, 255 } };

    case entity_type_enemy:
        return (Entity) { entity_type_enemy, x, y, 10, 10, (SDL_Color) { 255, 0, 0, 255 } };
    }

    return (Entity) { entity_type_empty, x, y, -1, -1, (SDL_Color) { 0, 0, 0, 0 } };
}

void destroy_entity(Entity* entity) {
    Entity empty_entity = new_entity(entity_type_empty, entity->x, entity->y);
    *grid[entity->y * MAP_WIDTH + entity->x] = empty_entity;
}

Entity* get_entity(int x, int y) {
    if ( x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return NULL;
    return grid[y * MAP_WIDTH + x];
}

bool set_entity(int x, int y, Entity* enity) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
    grid[y * MAP_WIDTH + x] = enity;
    return true;
}

bool spawn_entity(Entity entity) {
    if (entity.x < 0 || entity.y < 0 || entity.x >= MAP_WIDTH || entity.y >= MAP_HEIGHT) return false;
    Entity* old_entity = get_entity(entity.x, entity.y);
    if (old_entity->entity_type != entity_type_empty) return false;
    *old_entity = entity;
    if (entity.entity_type == entity_type_player) main_player = old_entity;
    return true;
}

void hit_entity(Entity* entity, int damage) {
    if (entity->max_health < 0) return;
    entity->health -= damage;
    if (entity->health <= 0) destroy_entity(entity);
}

bool switch_entities(int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 >= MAP_WIDTH || x2 >= MAP_WIDTH || y1 >= MAP_HEIGHT || y2 >= MAP_HEIGHT) return false;

    Entity* entity1 = get_entity(x1, y1);
    Entity* entity2 = get_entity(x2, y2);

    if (entity1->entity_type == entity_type_wall || entity2->entity_type == entity_type_wall) return false;

    if (entity1->entity_type == entity_type_player && entity2->max_health > 0) {
        hit_entity(entity2, 1);
        return false;
    }
    if (entity2->entity_type == entity_type_player && entity1->max_health > 0) {
        hit_entity(entity1, 1);
        return false;

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


//  TEXTURES

SDL_Texture* textures[entity_type_empty];

void load_textures() {
    for (int i = 0; i < entity_type_empty; i++) { textures[i] = NULL; }

    textures[entity_type_player] = IMG_LoadTexture(renderer, "./assets/player_texture.png");
    textures[entity_type_enemy] = IMG_LoadTexture(renderer, "./assets/enemy_texture.png");
    textures[entity_type_stone] = IMG_LoadTexture(renderer, "./assets/stone_texture.png");
}


//  CAMERA

typedef struct {
    int x;
    int y;
    float zoom;
} Camera;

Camera main_camera = { 0, 0, 1. };

void update_camera() {
    main_camera.x = main_player->x * TILE_SIZE + TILE_SIZE / 2 - SCREEN_WIDTH / 2;
    main_camera.y = main_player->y * TILE_SIZE + TILE_SIZE / 2 - SCREEN_HEIGHT / 2;
}


//  GRID

void reset_grids() {
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
        entities[i] = new_entity(entity_type_empty, i % MAP_WIDTH, i / MAP_WIDTH);
        grid[i] = &entities[i];
    }
}

void create_edge_walls() {
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) spawn_entity(new_entity(entity_type_wall, x, y));
}

void update_world(int player_movement_x, int player_movement_y) {
    if (player_movement_x == 0 && player_movement_y == 0) return;
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++)
    {
        Entity* entity = &entities[i];
        if (entity->entity_type == entity_type_empty) {}
        else if (entity->entity_type == entity_type_wall) {}
        else if (entity->entity_type == entity_type_player) {
            if (player_movement_x != 0)
            switch_entities(entity->x, entity->y, entity->x + player_movement_x, entity->y);
            if (player_movement_y != 0)
            switch_entities(entity->x, entity->y, entity->x, entity->y + player_movement_y);
        }
        else if (entity->entity_type == entity_type_enemy) {}
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
            entity = *get_entity(x, y);
            if (entity.entity_type != entity_type_empty) {
                if (entity.entity_type != entity_type_empty) {
                    tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y, TILE_SIZE, TILE_SIZE };
                    if (textures[entity.entity_type]) {
                        SDL_RenderCopy(renderer, textures[entity.entity_type], NULL, &tile);
                    }
                    else {
                        tile_color = entity.color;
                        SDL_SetRenderDrawColor(renderer, 255 * ((float)y / MAP_HEIGHT), 0, 255 * ((float)x / MAP_WIDTH), 255);
                        SDL_RenderFillRect(renderer, &tile);
                    }
                }
            }
        }
    }

    // GUI

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            entity = *get_entity(x, y);
            if (entity.entity_type != entity_type_empty) {
                if (entity.max_health > 0 && entity.max_health != entity.health) {
                    int max_height = 4;
                    int max_width = 20;
                    int tile_x = entity.x * TILE_SIZE - max_width / 2 + TILE_SIZE / 2;
                    int tile_y = entity.y * TILE_SIZE;
                    SDL_Rect background_rect = { (tile_x - 1), (tile_y - 1), (max_width + 1), (max_height + 1) };
                    SDL_Rect health_rect = { tile_x, tile_y, (max_width * ((float)entity.health / entity.max_health)), max_height };
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
                    SDL_RenderFillRect(renderer, &background_rect);
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200);
                    SDL_RenderFillRect(renderer, &health_rect);
                }
            }
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderClear(renderer);


    SDL_Rect screen_rect = { -((main_camera.x + SCREEN_WIDTH / 2) - (SCREEN_WIDTH / (2 * main_camera.zoom))) * main_camera.zoom, -((main_camera.y + SCREEN_HEIGHT / 2) - (SCREEN_HEIGHT / (2 * main_camera.zoom))) * main_camera.zoom, SCREEN_WIDTH * main_camera.zoom, SCREEN_HEIGHT * main_camera.zoom };

    //SDL_Rect camera_viewport = { , , SCREEN_WIDTH / main_camera.zoom, SCREEN_HEIGHT / main_camera.zoom };

    SDL_RenderCopy(renderer, screen, NULL, &screen_rect);
    SDL_RenderPresent(renderer);
}


void init_rendering() {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
}

int main(void) {
    
    reset_grids();
    create_edge_walls();

    //  Testing setup
    
    spawn_entity(new_entity(entity_type_player, 2, 1));
    spawn_entity(new_entity(entity_type_wall, 3, 4));
    spawn_entity(new_entity(entity_type_enemy, 2, 4));


    for (int i = 0; i < 10; i++) {
        spawn_entity(new_entity(entity_type_stone, 2 + i, 6));
        spawn_entity(new_entity(entity_type_stone, 3 + i, 7));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 8));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 9));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 10));
    }

    //

    if (!main_player) {
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
        if (window == NULL) {
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
                        else if (SDLK_e == event.key.keysym.sym) main_camera.zoom += .1;
                        else if (SDLK_q == event.key.keysym.sym) main_camera.zoom -= .1;
                        else if (SDLK_LEFT == event.key.keysym.sym) main_camera.x -= 10;
                        else if (SDLK_RIGHT == event.key.keysym.sym) main_camera.x += 10;
                        else if (SDLK_UP == event.key.keysym.sym) main_camera.y -= 10;
                        else if (SDLK_DOWN == event.key.keysym.sym) main_camera.y += 10;
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

    SDL_DestroyWindow(window);


    for (int i = 0; i < entity_type_empty; i++) {
        SDL_DestroyTexture(textures[i]);
    }


    IMG_Quit();
    SDL_Quit();

    return 0;
}