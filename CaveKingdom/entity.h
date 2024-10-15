#pragma once
#include <stdbool.h>

#include "inventory.h"
#include "entity_component.h"

struct Entity;

typedef struct Entity {
    int id;
    EntityType type;
    HeightLayer height_layer;
    int x, y;
    bool is_obstacle;
    int is_transparent; // int because -1 is unset
    struct Entity* connected_to;
    Combat combat;
    Hunger hunger;
    Health health;
    Brain brain;
    Inventory inventory;
    DemeanorType demeanor;
} Entity;


bool is_empty_entity_type(EntityType entity_type);
Entity new_entity(EntityType type, int x, int y);
void destroy_entity(Entity* entity);
Entity* get_entity(int x, int y, HeightLayer layer);
bool set_entity(int x, int y, Entity* entity);
bool spawn_entity(Entity entity);
bool force_spawn_entity(Entity entity);
void hit_entity(Entity* hitter, Entity* target);
//bool switch_entities(int x1, int y1, int x2, int y2, HeightLayer layer);

void reset_grids();
void create_edge_walls();
void update_entities();
bool update_player();

extern Entity* main_player;
extern bool main_player_alive;
extern const EntityType empty_entity_types[number_of_height_layers];
