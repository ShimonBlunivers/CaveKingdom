#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#include "entity.h"
#include "chunk.h"
#include "inventory.h"

Entity* main_player = NULL;

const EntityType empty_entity_types[number_of_height_layers] = {
    entity_type_ground_empty,
    entity_type_surface_empty,
    entity_type_air_empty,
}; // List of empty types for every layer, ascending.


Entity entity_list[MAP_WIDTH * MAP_HEIGHT * number_of_height_layers];
Entity* entity_position_grid[MAP_WIDTH * MAP_HEIGHT][number_of_height_layers]; // For quicker access through coordinates


bool is_empty_entity_type(EntityType entity_type) {
    for (int i = 0; i < number_of_height_layers; i++) if (entity_type == empty_entity_types[i]) return true;
    return false;
}

Entity new_entity(EntityType type, int x, int y) {
    HeightLayer height_layer;

    ItemStack loot[INVENTORY_SIZE] = { 0 };

    Entity new_entity = {
        type,
        -1,
        x, y,
        false,
        (Combat) { 0, 0 },
        (Hunger) { -1 },
        (Health) { -1, -1 },
        (Brain) { false, 50, (Vector2f){ 0., 0. } },
        0,
        demeanor_type_neutral,
    };



    bool entity_struct_initiated = false;
    new_entity.height_layer = height_layer_ground;    
    switch (type) {
    case entity_type_water:
        entity_struct_initiated = true;
        new_entity.is_obstacle = true;
        break;

    case entity_type_dirt:
        entity_struct_initiated = true;
        break;

    case entity_type_ground_empty:
        entity_struct_initiated = true;
        break;
    }

    if (!entity_struct_initiated) new_entity.height_layer = height_layer_surface;

    switch (type) {
    case entity_type_player:
        entity_struct_initiated = true;
        new_entity.health.max = 10;
        new_entity.is_obstacle = true;
        new_entity.combat.damage = 1;
        new_entity.hunger.saturation = 100;
        break;

    case entity_type_enemy:
        entity_struct_initiated = true;
        new_entity.brain.active = true;
        new_entity.health.max = 10;
        new_entity.is_obstacle = true;
        new_entity.combat.damage = 1;
        new_entity.hunger.saturation = 100;
        break;

    case entity_type_zombie:
        entity_struct_initiated = true;
        new_entity.brain.active = true;
        loot[0] = (ItemStack){ item_type_zombie_meat, 2 };
        new_entity.health.max = 10;
        new_entity.is_obstacle = true;
        new_entity.combat.damage = 1;
        new_entity.hunger.saturation = 100;
        break;

    case entity_type_wall:
        entity_struct_initiated = true;
        new_entity.is_obstacle = true;
        break;

    case entity_type_stone:
        entity_struct_initiated = true;
        loot[0] = (ItemStack){ item_type_stone, 3 };
        new_entity.health.max = 5;
        new_entity.is_obstacle = true;
        break;

    case entity_type_surface_empty:
        entity_struct_initiated = true;
        break;
    }

    if (!entity_struct_initiated) new_entity.height_layer = height_layer_air;
    switch (type) {}


    new_entity.inventory = new_inventory(loot);
    if (new_entity.health.value == -1) new_entity.health.value = new_entity.health.max;
    return new_entity;
}

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

void hit_entity(Entity* hitter, Entity* target) {
    if (hitter->combat.damage <= 0 || target->health.max < 0 ) return;
    int damage = hitter->combat.damage - target->combat.armor;

    target->health.value -= SDL_max(damage, 0);
    if (target->health.value <= 0) { 
        if (hitter) collect_inventory(&target->inventory, &hitter->inventory);
        destroy_entity(target); 
    }
}

void switch_entities(Entity* entity1, Entity* entity2) {
    int x1 = entity1->x;
    int y1 = entity1->y;

    int x2 = entity2->x;
    int y2 = entity2->y;

    entity2->x = entity1->x;
    entity2->y = entity1->y;

    entity1->x = x2;
    entity1->y = y2;

    Entity* temp = entity1;
    set_entity(x1, y1, entity2);
    set_entity(x2, y2, temp);
}

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

bool move_entity(Entity* entity, int x, int y) {
    if (x == 0 && y == 0) return false;
    if (entity->x + x < 0 || entity->y + y < 0 || entity->x + x >= MAP_WIDTH || entity->y + y >= MAP_HEIGHT) return false;
    Entity* neigbour = get_entity(entity->x + x, entity->y + y, entity->height_layer);
    for (int layer = 0; layer < number_of_height_layers; layer++) {
        if (layer == entity->height_layer) continue;
        if (get_entity(entity->x + x, entity->y + y, layer)->is_obstacle) return false;
    }
    
    if (neigbour->type != empty_entity_types[neigbour->height_layer]) {
        hit_entity(entity, neigbour);
        return false;
    }

    switch_entities(entity, neigbour);
    return true;
}

int random_direction() {
    return 1 - rand()%3;
}

float get_movement_randomisation() {
    return .9 + ((rand() % 3)) / 10.;
}

void update_entities(int player_movement_x, int player_movement_y) {
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT * number_of_height_layers; i++)
    {
        Entity* entity = &entity_list[i];
        if (entity->brain.active ) {


            for (int j = 0; j < 4; j++) {
                int x = j < 2 ? -1 + (j % 2) * 2 : 0;
                int y = j >= 2 ? -1 + ((j + 2) % 2) * 2 : 0;
                Entity* neighbour = get_entity(entity->x + x, entity->y + y, entity->height_layer);
                if (neighbour->health.value > 0) {
                    entity->brain.desired_direction = vector2f_sum(entity->brain.desired_direction, (Vector2f) { x * .5 , y * .5 });
                }
                else if (neighbour->is_obstacle || get_entity(entity->x + x, entity->y + y, entity->height_layer -1)->is_obstacle) {
                    entity->brain.desired_direction = vector2f_sum(entity->brain.desired_direction, (Vector2f) { -x * .2, -y * .2 });
                }
            }

            entity->brain.desired_direction = vector2f_sum(entity->brain.desired_direction, (Vector2f) { -.1 + ((rand() % 3)) / 10., -.1 + ((rand() % 3)) / 10. });
            
            entity->brain.desired_direction.x = SDL_clamp(entity->brain.desired_direction.x, -1., 1.);
            entity->brain.desired_direction.y = SDL_clamp(entity->brain.desired_direction.y, -1., 1.);

            if (entity->brain.desired_direction.x > 0.5 * get_movement_randomisation())
                move_entity(entity, 1, 0);
            else if (entity->brain.desired_direction.x < -0.5)
                move_entity(entity, -1, 0);
            if (entity->brain.desired_direction.y > 0.5)
                move_entity(entity, 0, 1);
            else if (entity->brain.desired_direction.y < -0.5 * get_movement_randomisation())
                move_entity(entity, 0, -1);
        }
        switch (entity->type) {
            case entity_type_player:
                if (player_movement_x != 0)
                    move_entity(entity, player_movement_x, 0);
                if (player_movement_y != 0)
                    move_entity(entity, 0, player_movement_y);
                break;

            case entity_type_zombie:
            case entity_type_enemy:
                break;
        }
    }
}

