#pragma once
#include <stdbool.h>

#define INVENTORY_SIZE 8

typedef enum {
    item_type_empty,

    item_type_stone,
    item_type_zombie_meat,

    number_of_item_types, // DO NOT USE AS ITEM TYPE!
} ItemType;

typedef struct {
    ItemType type;
    int amount;
} ItemStack;

typedef struct {
    ItemStack content[INVENTORY_SIZE];
} Inventory;


Inventory new_inventory(ItemStack content[INVENTORY_SIZE]);
void collect_inventory(Inventory* from, Inventory* to);
void print_inventory(Inventory inventory);