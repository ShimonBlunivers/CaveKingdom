#pragma once
#include <stdbool.h>

#include "item.h"

#define INVENTORY_SIZE 8
#define INVENTORY_HOTBAR_SLOTS 8

typedef struct Inventory {
    int size;
    int selected_slot;
    ItemStack content[];
} Inventory;


void select_inventory_slot(Inventory* inventory, int slot_index);
bool is_empty_inventory(Inventory inventory);
Inventory* new_inventory(int size);
bool add_to_inventory(Inventory* inventory, ItemStack item_stack);
void free_inventory(Inventory* inventory);
void collect_inventory(Inventory* from, Inventory* to);
void print_inventory(Inventory inventory);