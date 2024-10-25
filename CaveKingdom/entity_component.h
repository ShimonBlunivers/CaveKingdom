#pragma once

#include <stdlib.h>

#include "entity.h"
#include "vector.h"


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

    // Air types

    number_of_entity_types, // DO NOT USE AS ENTITY TYPE !
} EntityType;

typedef enum {
    height_layer_ground,
    height_layer_surface,
    height_layer_air,

    number_of_height_layers, // DO NOT USE AS HEIGHT LAYER !
} HeightLayer;

typedef enum {
    demeanor_type_neutral,
    demeanor_type_hostile,
    demeanor_type_friendly
} DemeanorType;

typedef struct {
    int value, max;
} Health;

typedef struct {
    int saturation;
} Hunger;

typedef struct {
    int damage, armor;
} Combat;

typedef struct {
    bool active;
    int mood;
    Vector2f desired_direction;
} Brain;

typedef struct {
    bool seen;
    int last_seen;
    struct Entity_struct* last_seen_as;
} Visibility;