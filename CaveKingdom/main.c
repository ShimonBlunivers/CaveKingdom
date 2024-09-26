#include <stdio.h>
#include <stdlib.h>
#include <SDL_image.h>
#include <SDL.h>

#define MAP_WIDTH 36
#define MAP_HEIGHT 36
#define TILE_SIZE 20

const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 720;

int game_status = 1;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

typedef struct {
    SDL_Texture* player_texture;
    SDL_Texture* enemy_texture;
    SDL_Texture* stone_texture;
} Textures;

typedef enum {
    entity_type_empty = 0,
    entity_type_player = 1,
    entity_type_wall = 2,
    entity_type_enemy = 4,
    entity_type_stone = 8,
} EntityType;

typedef struct {
    EntityType entity_type;
    int x;
    int y;
    int max_health;
    int health;
    SDL_Color color;
} Entity;

typedef struct {
    int x;
    int y;
    float zoom;
} Camera;

Camera main_camera = {0, 0, 1.};


static Entity entities[MAP_WIDTH * MAP_HEIGHT];
static Entity* grid[MAP_WIDTH * MAP_HEIGHT];

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

int set_entity(int x, int y, Entity* enity) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return 0;
    grid[y * MAP_WIDTH + x] = enity;
    return 1;
}

int spawn_entity(Entity entity) {
    if (entity.x < 0 || entity.y < 0 || entity.x >= MAP_WIDTH || entity.y >= MAP_HEIGHT) return 0;
    Entity* old_entity = get_entity(entity.x, entity.y);
    if (old_entity->entity_type != entity_type_empty) return 0;
    *old_entity = entity;
    return 1;
}

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


void hit_entity(Entity* entity, int damage) {
    if (entity->max_health < 0) return;
    entity->health -= damage;
    if (entity->health <= 0) destroy_entity(entity);
}

int switch_entities(int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 >= MAP_WIDTH || x2 >= MAP_WIDTH || y1 >= MAP_HEIGHT || y2 >= MAP_HEIGHT) return 0;

    Entity* entity1 = get_entity(x1, y1);
    Entity* entity2 = get_entity(x2, y2);

    if (entity1->entity_type == entity_type_wall || entity2->entity_type == entity_type_wall) return 0;

    if (entity1->entity_type == entity_type_player && entity2->max_health > 0) {
        hit_entity(entity2, 1);
        return 0;
    }
    if (entity2->entity_type == entity_type_player && entity1->max_health > 0) {
        hit_entity(entity1, 1);
        return 0;

    }

    entity2->x = x1;
    entity2->y = y1;

    entity1->x = x2;
    entity1->y = y2;

    Entity* temp = entity1;
    set_entity(x1, y1, entity2);
    set_entity(x2, y2, temp);

    return 1;
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

void draw_world(Textures textures) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderClear(renderer);

    Entity entity;

    SDL_Rect tile;
    SDL_Color tile_color;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            entity = *get_entity(x, y);
            if (entity.entity_type != entity_type_empty) {
                tile = (SDL_Rect){ (TILE_SIZE * x + main_camera.x) * main_camera.zoom, (TILE_SIZE * y + main_camera.y) * main_camera.zoom, TILE_SIZE * main_camera.zoom, TILE_SIZE * main_camera.zoom };
                if (entity.entity_type == entity_type_player) {
                    SDL_RenderCopy(renderer, textures.player_texture, NULL, &tile);
                }
                else if (entity.entity_type == entity_type_enemy) {
                    SDL_RenderCopy(renderer, textures.enemy_texture, NULL, &tile);
                }
                else if (entity.entity_type == entity_type_stone) {
                    SDL_RenderCopy(renderer, textures.stone_texture, NULL, &tile);
                }
                else {
                    tile_color = entity.color;
                    //SDL_SetRenderDrawColor(renderer, tile_color.r, tile_color.g, tile_color.b, tile_color.a);
                    SDL_SetRenderDrawColor(renderer, 255 * ((float)y / MAP_HEIGHT), 0, 255 * ((float)x / MAP_WIDTH), 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
            }
        }
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            entity = *get_entity(x, y);
            if (entity.entity_type != entity_type_empty) {
                if (entity.max_health > 0 && entity.max_health != entity.health) {
                    int max_height = 4;
                    int max_width = 20;
                    int tile_x = entity.x * TILE_SIZE - max_width / 2 + TILE_SIZE / 2;
                    int tile_y = entity.y * TILE_SIZE;
                    SDL_Rect background_rect = { (tile_x - 1) * main_camera.zoom, (tile_y - 1) * main_camera.zoom, (max_width + 1) * main_camera.zoom, (max_height + 1) * main_camera.zoom };
                    SDL_Rect health_rect = {tile_x * main_camera.zoom, tile_y * main_camera.zoom, (max_width * ((float) entity.health / entity.max_health)) * main_camera.zoom, max_height * main_camera.zoom };
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
                    SDL_RenderFillRect(renderer, &background_rect);
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200);
                    SDL_RenderFillRect(renderer, &health_rect);
                }
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void load_textures(Textures* textures) {
    textures->player_texture = IMG_LoadTexture(renderer, "./assets/player_texture.png");
    textures->enemy_texture = IMG_LoadTexture(renderer, "./assets/enemy_texture.png");
    textures->stone_texture = IMG_LoadTexture(renderer, "./assets/stone_texture.png");
}

int main(void) {

     reset_grids();
     create_edge_walls();

     Textures textures;

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


    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }
    else {
        window = SDL_CreateWindow("CaveKingdom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

            load_textures(&textures);

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
                        else if (SDLK_e == event.key.keysym.sym) main_camera.zoom += 0.1;

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
                draw_world(textures);

                SDL_Delay(100);
            }
        }
    }

    SDL_DestroyWindow(window);

    SDL_DestroyTexture(textures.enemy_texture);
    SDL_DestroyTexture(textures.player_texture);
    SDL_DestroyTexture(textures.stone_texture);

    IMG_Quit();
    SDL_Quit();

    return 0;
}