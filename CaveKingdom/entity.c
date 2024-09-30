#include <stdio.h>
#include <stdbool.h>

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

    Entity new_entity = { 0 };
    ItemStack loot[INVENTORY_SIZE] = { 0 };

    height_layer = height_layer_ground;
    switch (type) {
    case entity_type_water:
        new_entity = (Entity){ type, height_layer, x, y, -1, NULL, true, new_inventory(loot) };
        break;

    case entity_type_dirt:
        new_entity = (Entity){ type, height_layer, x, y, -1, NULL, false, new_inventory(loot) };
        break;

    case entity_type_ground_empty:
        new_entity = (Entity){ type, height_layer, x, y, -1, NULL, false, new_inventory(loot) };
        break;
    }

    height_layer = height_layer_surface;
    switch (type) {
    case entity_type_player:
        new_entity = (Entity){ type, height_layer, x, y, 10, NULL, true, new_inventory(loot) };
        break;

    case entity_type_enemy:
        new_entity = (Entity){ type, height_layer, x, y, 10, NULL, true, new_inventory(loot) };
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

void update_entities(int player_movement_x, int player_movement_y) {
    if (player_movement_x == 0 && player_movement_y == 0) return;
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT * number_of_height_layers; i++)
    {
        Entity* entity = &entity_list[i];
        switch (entity->type) {
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