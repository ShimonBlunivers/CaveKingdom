#pragma once

#include <stdlib.h>

#include "entities/entity.h"
#include "graphics/vector.h"


typedef enum EntityType {
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
	entity_type_skeleton_corpse,

	// Air types



	// Special types

	entity_type_dropped_items,

	number_of_entity_types, // DO NOT USE AS ENTITY TYPE !
} EntityType;

typedef enum HeightLayer {
	height_layer_ground,
	height_layer_surface,
	height_layer_air,

	number_of_height_layers, // DO NOT USE AS HEIGHT LAYER !
} HeightLayer;

typedef enum DemeanorType {
	demeanor_type_neutral,
	demeanor_type_hostile,
	demeanor_type_friendly
} DemeanorType;

typedef struct Health {
	int current, max;
} Health;

typedef struct Hunger {
	int saturation;
} Hunger;

typedef struct Combat {
	int damage, armor;
} Combat;

typedef struct Brain {
	bool active;
	int mood;
	Vector2f desired_direction;
	DemeanorType demeanor;
} Brain;

typedef struct Visibility {
	bool seen;
	Uint32 last_seen;
	struct Entity* last_seen_as;
} Visibility;

typedef struct Thermal {
	float temperature; // in kelvins
	float max_temperature; // -1 means no limit
	float min_temperature; // -1 means no limit
	float conductivity; // 0 means isolant; 1 let's everything through

	float generated_heat_per_tick;
} Thermal;