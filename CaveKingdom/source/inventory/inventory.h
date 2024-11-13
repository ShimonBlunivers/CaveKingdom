#pragma once
#include <stdbool.h>

#define INVENTORY_SIZE 8

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

typedef struct Inventory {
    int size;
    ItemStack content[INVENTORY_SIZE];
    int selected_slot;
} Inventory;


void select_inventory_slot(Inventory* inventory, int slot_index);
Inventory get_empty_inventory();
bool is_empty_inventory(Inventory inventory);
Inventory new_inventory(ItemStack content[INVENTORY_SIZE]);
void collect_inventory(Inventory* from, Inventory* to);
void print_inventory(Inventory inventory);