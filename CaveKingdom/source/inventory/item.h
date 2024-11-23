#pragma once

typedef enum ItemType {
    item_type_empty,

    item_type_stone,
    item_type_wood,
    item_type_zombie_meat,

    number_of_item_types, // DO NOT USE AS ITEM TYPE!
} ItemType;

typedef struct ItemStack {
    ItemType type;
    int amount;
} ItemStack;