#pragma once
#include <stdbool.h>

#include "inventory.h"

typedef enum {
    demeanor_type_neutral,
    demeanor_type_hostile,
    demeanor_type_friendly
} DemeanorType;

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

typedef enum {
    height_layer_ground,
    height_layer_surface,
    height_layer_air,

    number_of_height_layers, // DO NOT USE AS HEIGHT LAYER !
} HeightLayer;

typedef struct {
    EntityType type;
    HeightLayer height_layer;
    int x;
    int y;
    int max_health;
    int health;
    int damage;
    bool is_obstacle;
    DemeanorType demeanor;
    bool is_alive;
    Inventory inventory;
} Entity;


bool is_empty_entity_type(EntityType entity_type);
Entity new_entity(EntityType type, int x, int y);
void destroy_entity(Entity* entity);
Entity* get_entity(int x, int y, HeightLayer layer);
bool set_entity(int x, int y, Entity* entity);
bool spawn_entity(Entity entity);
bool force_spawn_entity(Entity entity);
void hit_entity(Entity* entity, int damage);
bool switch_entities(int x1, int y1, int x2, int y2, HeightLayer layer);
void reset_grids();
void create_edge_walls();
void update_entities(int player_movement_x, int player_movement_y);

extern Entity* main_player;
extern const EntityType empty_entity_types[number_of_height_layers];
