#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#include "entities/entity.h"
#include "world/chunk.h"
#include "inventory/inventory.h"
#include "input/input.h"

#include "world/perlin.h"

#include "graphics/animation.h"
#include "world/time.h"
#include "graphics/particles.h"

Entity* main_player = NULL;
bool main_player_alive = true;
int number_of_entities = 0;

const EntityType empty_entity_types[number_of_height_layers] = {
    entity_type_ground_empty,
    entity_type_surface_empty,
    entity_type_air_empty,
}; // List of empty types for every layer, ascending.


Entity entity_list[CHUNK_WIDTH * CHUNK_HEIGHT * number_of_height_layers];
Entity* entity_position_grid[CHUNK_WIDTH * CHUNK_HEIGHT][number_of_height_layers]; // For quicker access through coordinates


bool is_empty_entity_type(EntityType entity_type) {
    for (int i = 0; i < number_of_height_layers; i++) if (entity_type == empty_entity_types[i]) return true;
    return false;
}

Entity new_entity(EntityType type, int x, int y) {
    HeightLayer height_layer;

    ItemStack loot[INVENTORY_SIZE] = { (ItemStack) { item_type_empty, 0 } };

    Entity new_entity = {
        .id = number_of_entities++,
        .type = type,
        .color = (SDL_Color){ 128, 128, 128, 255 },
        .height_layer = height_layer_ground,
        .x = x,
        .y = y,
        .is_obstacle = false,
        .is_transparent = -1,
        .rotation = 0,
        .visibility = malloc(sizeof(Visibility)),
        .health = malloc(sizeof(Health)),

        .hunger = NULL,
        .combat = NULL,
        .brain = NULL,
        .inventory = NULL,

        .tween = NULL,
    };

    bool has_inventory = false; // Set this to true if the entity has inventory but doesn't have any loot.
    bool randomize_rotation = false;

    if (new_entity.visibility != NULL) *new_entity.visibility = (Visibility){ false, false, -1 };
    else printf("Error: new_entity.visibility was NULL when creating new entity.\n");
    if (new_entity.health != NULL) *new_entity.health = (Health){ -1, -1 };
    else printf("Error: new_entity.health was NULL when creating new entity.\n");

    bool entity_struct_initiated = false;
    new_entity.height_layer = height_layer_ground;   

    switch (type) {
    case entity_type_water:
        entity_struct_initiated = true;

        new_entity.color = (SDL_Color){ 144, 203, 239, 255 };

        new_entity.is_obstacle = true;
        break;

    case entity_type_dirt:
        entity_struct_initiated = true;
        randomize_rotation = true;

        new_entity.color = (SDL_Color){ 89, 60, 44, 255 };

        break;

    case entity_type_ground_empty:
        entity_struct_initiated = true;
        break;
    }

    if (!entity_struct_initiated) new_entity.height_layer = height_layer_surface;

    switch (type) {
    case entity_type_player:
        entity_struct_initiated = true;

        new_entity.color = (SDL_Color){ 217, 149, 149, 255 };

        new_entity.combat = (Combat*)malloc(sizeof(Combat));
        new_entity.hunger = (Hunger*)malloc(sizeof(Hunger));

        if (new_entity.combat != NULL) *new_entity.combat = (Combat){ 1, 0 };
        if (new_entity.hunger != NULL) *new_entity.hunger = (Hunger){100};

        has_inventory = true;
        new_entity.health->max = 10;
        new_entity.is_obstacle = true;
        new_entity.is_transparent = true;
        break;

    case entity_type_enemy:
        entity_struct_initiated = true;

        new_entity.color = (SDL_Color){ 217, 149, 149, 255 };

        new_entity.brain = (Brain*)malloc(sizeof(Brain));
        new_entity.combat = (Combat*)malloc(sizeof(Combat));
        new_entity.hunger = (Hunger*)malloc(sizeof(Hunger));

        if (new_entity.brain != NULL) *new_entity.brain = (Brain){ true, 50, (Vector2f) { 0., 0. } };
        if (new_entity.combat != NULL) *new_entity.combat = (Combat){ 1, 0 };
        if (new_entity.hunger != NULL) *new_entity.hunger = (Hunger){ 100 };

        has_inventory = true;
        new_entity.health->max = 10;
        new_entity.is_obstacle = true;
        new_entity.is_transparent = true;
        break;

    case entity_type_zombie:
        entity_struct_initiated = true;

        new_entity.color = (SDL_Color){ 137, 172, 140, 255 };

        new_entity.brain = (Brain*)malloc(sizeof(Brain));
        new_entity.combat = (Combat*)malloc(sizeof(Combat));
        new_entity.hunger = (Hunger*)malloc(sizeof(Hunger));

        if (new_entity.brain != NULL) *new_entity.brain = (Brain){ true, 50, (Vector2f) { 0., 0. } };
        if (new_entity.combat != NULL) *new_entity.combat = (Combat){ 1, 0 };
        if (new_entity.hunger != NULL) *new_entity.hunger = (Hunger){ 100 };

        loot[0] = (ItemStack){ item_type_zombie_meat, 2 };
        new_entity.health->max = 10;
        new_entity.is_obstacle = true;
        new_entity.is_transparent = true;
        break;

    case entity_type_wall:
        entity_struct_initiated = true;

        new_entity.is_obstacle = true;
        break;

    case entity_type_stone:
        entity_struct_initiated = true;

        new_entity.color = (SDL_Color){ 97, 97, 97, 255 };

        loot[0] = (ItemStack){ item_type_stone, 3 };
        new_entity.health->max = 5;
        new_entity.is_obstacle = true;
        break;

    case entity_type_surface_empty:
        entity_struct_initiated = true;
        break;
    }

    if (!entity_struct_initiated) new_entity.height_layer = height_layer_air;
    switch (type) {

    case entity_type_air_empty:
        entity_struct_initiated = true;
        break;
    }

    Inventory inventory = new_inventory(loot);
    if (!is_empty_inventory(inventory) || has_inventory) {
        new_entity.inventory = (Inventory*)malloc(sizeof(Inventory));
        if (new_entity.inventory != NULL) *new_entity.inventory = inventory;
        else printf("Error: new_entity.inventory was NULL when creating new entity.\n");
    }

    if (randomize_rotation) new_entity.rotation = rand() % 5;
    if (new_entity.health->value == -1) new_entity.health->value = new_entity.health->max;
    if (new_entity.is_transparent == -1) new_entity.is_transparent = !new_entity.is_obstacle;

    return new_entity;
}

void free_entity(Entity* entity) {
    free(entity->visibility);
    free(entity->combat);
    free(entity->hunger);
    free(entity->health);
    free(entity->brain);
    free(entity->inventory);
    delete_tween(entity->tween);
}

void destroy_entity(Entity* entity) {
    if (entity->id == main_player->id) main_player_alive = false;

    free_entity(entity);

    Entity empty_entity = new_entity(empty_entity_types[entity->height_layer], entity->x, entity->y);

    for (int layer = 0; layer < number_of_height_layers; layer++) {
        if (layer == empty_entity.height_layer) continue;
        Entity* e = get_entity(empty_entity.x, empty_entity.y, layer);
        if (e->visibility == NULL) continue;
        e->visibility->seen = false;
        e->visibility->last_seen_as = NULL;
    }

    *entity_position_grid[entity->y * CHUNK_WIDTH + entity->x][empty_entity.height_layer] = empty_entity;
}

Entity* get_entity(int x, int y, HeightLayer layer) {
    if ( x < 0 || y < 0 || x >= CHUNK_WIDTH || y >= CHUNK_HEIGHT || layer >= number_of_height_layers || layer < 0) return NULL;
    return entity_position_grid[y * CHUNK_WIDTH + x][layer];
}

bool set_entity(int x, int y, Entity* enity) {
    if (x < 0 || y < 0 || x >= CHUNK_WIDTH || y >= CHUNK_HEIGHT) return false;
    entity_position_grid[y * CHUNK_WIDTH + x][enity->height_layer] = enity;
    return true;
}

bool spawn_entity(Entity entity) {
    if (entity.x < 0 || entity.y < 0 || entity.x >= CHUNK_WIDTH || entity.y >= CHUNK_HEIGHT) return false;
    Entity* entity_pointer = get_entity(entity.x, entity.y, entity.height_layer);
    if (!is_empty_entity_type(entity_pointer->type)) return false;
    *entity_pointer = entity;
    if (entity.type == entity_type_player) {
        main_player = entity_pointer;
    }
    return true;
}

bool force_spawn_entity(Entity entity) {
    if (entity.x < 0 || entity.y < 0 || entity.x >= CHUNK_WIDTH || entity.y >= CHUNK_HEIGHT) return false;
    Entity* old_entity = get_entity(entity.x, entity.y, entity.height_layer);
    free_entity(old_entity);
    *old_entity = entity;
    if (entity.type == entity_type_player) {
        main_player = old_entity;
        main_player = old_entity;
    }
    return true;
}

void hit_entity(Entity* hitter, Entity* target) {
    if (target == NULL || hitter->combat == NULL) return;
    if (hitter->combat->damage <= 0 || target->health->max < 0 ) return;
    int damage = hitter->combat->damage - ((target->combat != NULL) ? target->combat->armor : 0);
    target->health->value -= SDL_max(damage, 0);

    if (target->visibility->seen) for (int i = 0; i < 5; i++)
        new_particle((target->x + 0.25 + ((float)(rand() % 6)) / 10) * TILE_SIZE, (target->y + 0.25 + ((float)(rand() % 6)) / 10) * TILE_SIZE, target->color);

    if (target->health->value <= 0) { 
        if (target->inventory != NULL && hitter->inventory != NULL) collect_inventory(target->inventory, hitter->inventory);
        destroy_entity(target); 
    }
}

void switch_entities(Entity* entity1, Entity* entity2) {
    int x1 = entity1->x;
    int y1 = entity1->y;

    int x2 = entity2->x;
    int y2 = entity2->y;


    if (entity1->tween == NULL) {
        entity1->tween = new_tween(x1 * TILE_SIZE, y1 * TILE_SIZE, x2 * TILE_SIZE, y2 * TILE_SIZE, graphic_tick + move_tile_tween_duration);
    }
    else {
        *entity1->tween = change_finish_tween(*entity1->tween, (Vector2) { x2* TILE_SIZE, y2* TILE_SIZE });
    }

    if (entity2->tween == NULL) {
        entity2->tween = new_tween(x2 * TILE_SIZE, y2 * TILE_SIZE, x1 * TILE_SIZE, y1 * TILE_SIZE, graphic_tick + move_tile_tween_duration);
    }
    else {
        *entity2->tween = change_finish_tween(*entity2->tween, (Vector2) { x1* TILE_SIZE, y1* TILE_SIZE });
    }

    entity2->x = entity1->x;
    entity2->y = entity1->y;

    entity1->x = x2;
    entity1->y = y2;

    Visibility* temp_visibility = entity1->visibility;
    entity1->visibility = entity2->visibility;
    entity2->visibility = temp_visibility;


    Entity* temp_entity = entity1;
    set_entity(x1, y1, entity2);
    set_entity(x2, y2, temp_entity);
}

void reset_grids() {
    for (int layer = 0; layer < number_of_height_layers; layer++) {
        for (int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT; i++) {
            if (layer == height_layer_ground) entity_list[i + CHUNK_WIDTH * CHUNK_HEIGHT * layer] = new_entity(entity_type_dirt, i % CHUNK_WIDTH, i / CHUNK_WIDTH);
            if (layer == height_layer_surface) entity_list[i + CHUNK_WIDTH * CHUNK_HEIGHT * layer] = new_entity(entity_type_surface_empty, i % CHUNK_WIDTH, i / CHUNK_WIDTH);

            entity_position_grid[i][layer] = &entity_list[i + CHUNK_WIDTH * CHUNK_HEIGHT * layer];
        }
    }
}

void generate_world(int seed) {
    float freq = 10;
    float amp = 0.1;

    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            if (!(x == 0 || y == 0 || x == CHUNK_WIDTH - 1 || y == CHUNK_HEIGHT - 1)) {

                float noise = perlin((x + seed) * freq / CHUNK_WIDTH, (y + seed) * freq / CHUNK_HEIGHT) * amp;
                //printf("%.2f |", noise);
                if (noise > -0.1) force_spawn_entity(new_entity(entity_type_stone, x, y));
                else force_spawn_entity(new_entity(entity_type_surface_empty, x, y));
            }
        }
        //printf("\n");
    }
}

void create_edge_walls() {
    for (int y = 0; y < CHUNK_HEIGHT; y++)
        for (int x = 0; x < CHUNK_WIDTH; x++)
            if (x == 0 || y == 0 || x == CHUNK_WIDTH - 1 || y == CHUNK_HEIGHT - 1) {
                force_spawn_entity(new_entity(entity_type_water, x, y));
            }
}

bool is_tile_obstacle(int x, int y) {
    for (int layer = 0; layer < number_of_height_layers; layer++) {
        //if (layer == entity->height_layer) continue;
        if (get_entity(x, y, layer)->is_obstacle) return true;
    }
    return false;
}

Vector2 find_empty_tile() {
    int start_x = rand() % CHUNK_WIDTH;
    int start_y = rand() % CHUNK_HEIGHT;

    Vector2 spare_position = { -1, -1 };

    int ignore_positions = 5;

    int x, y;
    for (int _y = 0; _y < CHUNK_HEIGHT; _y++) {
        for (int _x = 0; _x < CHUNK_WIDTH; _x++) {
            x = (_x + start_x) % CHUNK_WIDTH;
            y = (_y + start_y) % CHUNK_HEIGHT;
            if (!is_tile_obstacle(x, y)) {
                if (ignore_positions-- == 0) return (Vector2){ x, y };
                spare_position = (Vector2){ x, y };
            }
        }
    }

    return spare_position;
}

void spawn_player() {

    Vector2 spawn_position = find_empty_tile();
    force_spawn_entity(new_entity(entity_type_player, spawn_position.x, spawn_position.y));
}

bool move_entity(Entity* entity, int x, int y) {
    if (x == 0 && y == 0) return false;
    if (entity->x + x < 0 || entity->y + y < 0 || entity->x + x >= CHUNK_WIDTH || entity->y + y >= CHUNK_HEIGHT) return false;

    Entity* neigbour = get_entity(entity->x + x, entity->y + y, entity->height_layer);

    bool collided = is_tile_obstacle(entity->x + x, entity->y + y);
    if (entity->type == entity_type_player && collided) printf("Collisions: x: %d , y: %d , layer: %d\n", entity->x + x, entity->y + y, entity->height_layer);
    if (collided) return false;

    switch_entities(entity, neigbour);

    return true;
}

int random_direction() {
    return 1 - rand()%3;
}

float get_movement_randomisation() {
    return .9 + ((rand() % 3)) / 10.;
}

void update_entities() {
    for (int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT * number_of_height_layers; i++) {
        Entity* entity = &entity_list[i];

        if (entity->brain != NULL && entity->brain->active) {
            for (int j = 0; j < 4; j++) {
                int x = j < 2 ? -1 + (j % 2) * 2 : 0;
                int y = j >= 2 ? -1 + ((j + 2) % 2) * 2 : 0;
                Entity* neighbour = get_entity(entity->x + x, entity->y + y, entity->height_layer);
                if (neighbour->health->value > 0) {
                    entity->brain->desired_direction = vector2f_sum(entity->brain->desired_direction, (Vector2f) { x * .5 , y * .5 });
                    hit_entity(entity, neighbour);
                }
                else if (neighbour->is_obstacle || get_entity(entity->x + x, entity->y + y, entity->height_layer -1)->is_obstacle) {
                    entity->brain->desired_direction = vector2f_sum(entity->brain->desired_direction, (Vector2f) { -x * .2, -y * .2 });
                }
            }

            entity->brain->desired_direction = vector2f_sum(entity->brain->desired_direction, (Vector2f) { -.1 + ((rand() % 3)) / 10., -.1 + ((rand() % 3)) / 10. });
            
            entity->brain->desired_direction.x = SDL_clamp(entity->brain->desired_direction.x, -1., 1.);
            entity->brain->desired_direction.y = SDL_clamp(entity->brain->desired_direction.y, -1., 1.);

            if (entity->brain->desired_direction.x > 0.5 * get_movement_randomisation())
                move_entity(entity, 1, 0);
            else if (entity->brain->desired_direction.x < -0.5)
                move_entity(entity, -1, 0);
            if (entity->brain->desired_direction.y > 0.5)
                move_entity(entity, 0, 1);
            else if (entity->brain->desired_direction.y < -0.5 * get_movement_randomisation())
                move_entity(entity, 0, -1);
        }

        switch (entity->type) {
            break;
            case entity_type_zombie:
            case entity_type_enemy:
                break;
        }
    }
}

void free_world() {
    for (int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT * number_of_height_layers; i++)
        free_entity(&entity_list[i]);
}

bool update_player() {
    bool updated = false;

    if (mouse.left_button_pressed) {
        Vector2 clicked_tile_position = from_screen_to_tile_coords((Vector2) { mouse.x, mouse.y });

        Vector2 distance = vector2_subtract(clicked_tile_position, (Vector2) { main_player->x, main_player->y });

        if (abs(distance.x) <= 1 && abs(distance.y) <= 1) {
            Entity* entity_clicked = get_entity(clicked_tile_position.x, clicked_tile_position.y, height_layer_surface);

            if (entity_clicked->type != entity_type_player) {
                hit_entity(main_player, entity_clicked);
                updated = true;
            }
        }
    }

    int player_movement_x = 0;
    int player_movement_y = 0;

    if (keyboard[key_w].active) player_movement_y--;
    if (keyboard[key_s].active) player_movement_y++;
    if (keyboard[key_a].active) player_movement_x--;
    if (keyboard[key_d].active) player_movement_x++;
    
    bool moved_x = false;
    bool moved_y = false;
    if (player_movement_x != 0) {
        moved_x = move_entity(main_player, player_movement_x, 0);
        updated = true;
    }
    if (player_movement_y != 0) {
        moved_y = move_entity(main_player, 0, player_movement_y);
        updated = true;
    }

    if (moved_x && moved_y) printf("moved diagonal\n");
    else if (moved_x) printf("moved X %d\n", moved_x);
    else if (moved_y) printf("moved Y %d\n", moved_y);
    
    if (player_movement_x || player_movement_y) printf("x: %d ; y: %d\n", player_movement_x, player_movement_y);

    return updated;
}