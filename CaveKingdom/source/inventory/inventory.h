#pragma once
#include <stdbool.h>

#include "item.h"

#define INVENTORY_HOTBAR_SLOT_COUNT 8

#define MAX_INVENTORY_SIZE 36

typedef struct Inventory {
	int size;
	int selected_slot;
	ItemStack content[];
} Inventory;


void select_inventory_slot(Inventory* inventory, int slot_index);
bool is_empty_inventory(Inventory inventory);
Inventory* new_inventory(int size);
int add_to_inventory(Inventory* inventory, ItemStack item_stack);
void free_inventory(Inventory* inventory);
void collect_inventory(Inventory* from, Inventory* to);
SDL_Rect get_player_slot_rect(Inventory* inventory, int i);

void print_inventory(Inventory inventory);

void update_player_inventory();

extern SDL_Rect player_inventory_rect;
extern SDL_Rect player_hotbar_rect;

extern ItemStack item_stack_held_by_mouse;