#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#define MAP_WIDTH 36
#define MAP_HEIGHT 36
#define TILE_SIZE 20

const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 720;

int game_status = 1;

typedef enum {
    entity_type_empty = 0,
    entity_type_player = 1,
    entity_type_wall = 2,
    entity_type_enemy = 4,
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

Entity new_empty(int x, int y) {
    return (Entity) { entity_type_empty, x, y, -1, -1, (SDL_Color) {0, 0, 0, 0} };
}

void reset_grids() {
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
        entities[i] = new_empty(i % MAP_WIDTH, i / MAP_WIDTH);
        grid[i] = &entities[i];
    }
}


Entity new_player(int x, int y) {
    return (Entity) { entity_type_player, x, y, 100, 100, (SDL_Color) { 0, 0, 255, 255 }};
}

Entity new_wall(int x, int y) {
    return (Entity) { entity_type_wall, x, y, -1, -1, (SDL_Color) { 32, 32, 32, 255 }};
}

Entity new_stone(int x, int y) {
    return (Entity) { entity_type_wall, x, y, 5, 5, (SDL_Color) { 64, 64, 64, 255 } };
}


Entity new_enemy(int x, int y) {
    return (Entity) { entity_type_enemy, x, y, 100, 100, (SDL_Color) { 255, 0, 0, 255 }};
}


Entity* get_entity(int x, int y) {
    return grid[y * MAP_WIDTH + x];
}

void set_entity(int x, int y, Entity* enity) {
    grid[y * MAP_WIDTH + x] = enity;
}

int spawn_entity(Entity entity) {
    if (entity.x < 0 || entity.y < 0 || entity.x >= MAP_WIDTH || entity.y >= MAP_HEIGHT) return 0;
    Entity* old_entity = get_entity(entity.x, entity.y);
    if (old_entity->entity_type != entity_type_empty) return 0;
    *old_entity = entity;
    return 1;
}

void create_edge_walls() {
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) spawn_entity(new_wall(x, y));
}

void hit_entity(Entity* entity, int damage) {
    if (entity->max_health < 0) return;
    entity->health -= damage;
    if (entity->health <= 0) spawn_entity(new_empty(entity->x, entity->y));
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

void draw_world(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    Entity entity;

    SDL_Rect tile;
    SDL_Color tile_color;
    int tile_size = 40;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            entity = *get_entity(x, y);
            if (entity.entity_type != entity_type_empty) {
                tile = (SDL_Rect){ TILE_SIZE * x, TILE_SIZE * y , TILE_SIZE, TILE_SIZE };
                tile_color = entity.color;
                SDL_SetRenderDrawColor(renderer, tile_color.r, tile_color.g, tile_color.b, tile_color.b);
                SDL_RenderFillRect(renderer, &tile);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {

     reset_grids();
     create_edge_walls();

     spawn_entity(new_player(2, 1));
     spawn_entity(new_enemy(3, 4));
     spawn_entity(new_wall(2, 4));

     spawn_entity(new_stone(2, 6));

    SDL_Window* window = NULL;

    SDL_Renderer* renderer = NULL;


    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        window = SDL_CreateWindow("CaveKingdom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

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
                draw_world(renderer);

                SDL_Delay(100);
            }
        }
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}