#pragma once
#include <SDL.h>
#include <stdbool.h>

#include "inventory/inventory.h"
#include "entities/entity_component.h"
#include "graphics/animation.h"

typedef struct Entity {
	int id;
	int x, y;
	bool is_obstacle;
	int rotation; // 0, 1, 2, 3
	int is_transparent; // int because -1 is unset

	Thermal thermal;
	EntityType type;
	SDL_Color color;
	HeightLayer height_layer;
	Combat* combat;
	Hunger* hunger;
	Health* health;
	Brain* brain;
	Inventory* inventory;
	Tween* tween;
	Visibility* visibility;

} Entity;

bool is_empty_entity_type(EntityType entity_type);
Entity new_entity(EntityType type, int x, int y);
void destroy_entity(Entity* entity);
void drop_items(int x, int y, ItemStack items);

Entity* get_entity(int x, int y, HeightLayer layer);

bool set_entity(int x, int y, Entity* entity);
bool spawn_entity(Entity entity);
void force_spawn_entity(Entity entity);
bool hit_entity(Entity* hitter, Entity* target);
Vector2 find_empty_tile();
//bool switch_entities(int x1, int y1, int x2, int y2, HeightLayer layer);

void spawn_player();

void update_entities();
bool update_player();

extern Entity* main_player;
extern bool main_player_alive;
extern bool main_player_updated;
extern const EntityType empty_entity_types[number_of_height_layers];