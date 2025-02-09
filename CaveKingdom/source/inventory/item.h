#pragma once

#define MAX_STACK_SIZE 64

typedef enum ItemType {
	item_type_empty,

	item_type_stone,
	item_type_wood,
	item_type_zombie_meat,
	item_type_bone,

	number_of_item_types, // DO NOT USE AS ITEM TYPE!
} ItemType;

typedef struct ItemStack {
	ItemType type;
	int amount;
} ItemStack;

void switch_stacks(ItemStack* item_stack_1, ItemStack* item_stack_2);
void move_stack(ItemStack* item_stack_from, ItemStack* item_stack_to);
void move_items(ItemStack* item_stack_from, ItemStack* item_stack_to, int amount);