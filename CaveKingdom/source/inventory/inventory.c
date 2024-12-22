#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "inventory.h"
#include "input/input.h"
#include "entities/entity.h"
#include "core/game.h"

SDL_Rect player_inventory_rect = {
	.x = (SCREEN_WIDTH - (351 * UI_RATIO)) / 2,
	.y = (SCREEN_HEIGHT - (314 * UI_RATIO)) / 2,
	.w = (351 * UI_RATIO),
	.h = (314 * UI_RATIO),
};

SDL_Rect player_hotbar_rect = {
	.x = (SCREEN_WIDTH - (int)(SCREEN_WIDTH * .9)) / 2,
	.y = SCREEN_HEIGHT - (int)(SCREEN_HEIGHT * .12) - 10,
	.w = (int)(SCREEN_WIDTH * .9),
	.h = (int)(SCREEN_HEIGHT * .12)
};

ItemStack item_stack_held_by_mouse = { item_type_empty, -1 };

// Adds item_stack to inventory. Returns the amount of items that could not be added.
int add_to_inventory(Inventory* inventory, ItemStack item_stack) {
	for (int i = 0; i < inventory->size; i++) {
		if (inventory->content[i].type == item_stack.type) {
			if (inventory->content[i].amount + item_stack.amount > MAX_STACK_SIZE) {
				item_stack.amount += inventory->content[i].amount - MAX_STACK_SIZE;
				inventory->content[i].amount = MAX_STACK_SIZE;
			}
			else {
				inventory->content[i].amount += item_stack.amount;
				return 0;
			}
		}
	}
	for (int i = 0; i < inventory->size; i++) {
		if (inventory->content[i].type == item_type_empty) {
			inventory->content[i] = item_stack;
			if (item_stack.amount > MAX_STACK_SIZE) {
				item_stack.amount -= MAX_STACK_SIZE;
				inventory->content[i].amount = MAX_STACK_SIZE;
			}
			else return 0;
		}
	}
	return item_stack.amount;
}

Inventory* new_inventory(int size) {
	if (size <= 0) {
		printf("Error: Inventory size must be greater than zero.\n");
		return NULL;
	}
	if (size > MAX_INVENTORY_SIZE) {
		printf("Error: Inventory size must be less than MAX_INVENTORY_SIZE.\n");
		return NULL;
	}

	Inventory* inventory = malloc(sizeof(Inventory) + size * sizeof(ItemStack));
	if (inventory == NULL) {
		printf("Error: Failed to allocate memory for inventory.\n");
		return NULL;
	}

	inventory->size = size;
	inventory->selected_slot = -1;

	for (int i = 0; i < size; i++) inventory->content[i] = (ItemStack){ item_type_empty, -1 };

	return inventory;
}

void free_inventory(Inventory* inventory) {
	free(inventory);
}

void select_inventory_slot(Inventory* inventory, int slot_index) {
	if (slot_index < 0 || slot_index > inventory->size) inventory->selected_slot = -1;
	inventory->selected_slot = slot_index;
}

void collect_inventory(Inventory* from, Inventory* to) {
	if (from == NULL || to == NULL) return;
	for (int i = 0; i < from->size; i++) {
		if (from->content[i].type == item_type_empty) continue;
		add_to_inventory(to, from->content[i]);
		from->content[i] = (ItemStack){ item_type_empty, -1 };
	}
}

bool is_empty_inventory(Inventory inventory) {
	for (int i = 0; i < inventory.size; i++) {
		if (inventory.content[i].type != item_type_empty) return false;
	}
	return true;
}

void print_inventory(Inventory inventory) {
	printf("------------\n");
	for (int i = 0; i < inventory.size; i++) {
		if (inventory.content[i].type != item_type_empty) {
			printf("%d : %d\n", inventory.content[i].type, inventory.content[i].amount);
		}
	}
}


SDL_Rect get_player_slot_rect(Inventory* inventory, int i) {
	if (inventory == NULL || i >= inventory->size) return (SDL_Rect) { 0, 0, 0, 0 };
	SDL_Rect slot_rect;
	if (inventory_opened) {
		int number_of_slots_per_row = 9;
		int slot_size = 37 * UI_RATIO;

		int x = i % number_of_slots_per_row;
		int y = i / number_of_slots_per_row;

		slot_rect = (SDL_Rect){ 9 * UI_RATIO + player_inventory_rect.x + slot_size * x, 9 * UI_RATIO + 3 * slot_size + player_inventory_rect.y + slot_size * (inventory->size / number_of_slots_per_row - y), slot_size, slot_size };
	}
	else {
		int slot_size = player_hotbar_rect.w / INVENTORY_HOTBAR_SLOT_COUNT;
		slot_rect = (SDL_Rect){ player_hotbar_rect.x + slot_size * i, player_hotbar_rect.y, slot_size, slot_size };
	}
	return slot_rect;
}

ItemStack* get_item_stack_on_position(Vector2 position) {

	for (int i = 0; i < main_player->inventory->size; i++) {
		SDL_Rect slot_rect = get_player_slot_rect(main_player->inventory, i);

		if (position.x >= slot_rect.x && position.x <= slot_rect.x + slot_rect.w && position.y >= slot_rect.y && position.y <= slot_rect.y + slot_rect.h) {
			return &main_player->inventory->content[i];
		}
	};
	return NULL;
}

void update_player_inventory() {
	if (main_player == NULL || !main_player_alive) return;

	if (key_tapped(keyboard[key_tab])) inventory_opened = !inventory_opened;

	if (key_tapped(keyboard[key_1])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 0 ? -1 : 0;
	else if (key_tapped(keyboard[key_2])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 1 ? -1 : 1;
	else if (key_tapped(keyboard[key_3])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 2 ? -1 : 2;
	else if (key_tapped(keyboard[key_4])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 3 ? -1 : 3;
	else if (key_tapped(keyboard[key_5])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 4 ? -1 : 4;
	else if (key_tapped(keyboard[key_6])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 5 ? -1 : 5;
	else if (key_tapped(keyboard[key_7])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 6 ? -1 : 6;
	else if (key_tapped(keyboard[key_8])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 7 ? -1 : 7;
	else if (key_tapped(keyboard[key_9])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 8 ? -1 : 8;
	else if (key_tapped(keyboard[key_0])) main_player->inventory->selected_slot = main_player->inventory->selected_slot == 9 ? -1 : 9;

	if (main_player->inventory) {
		if (mouse.left_button_clicked) {
			ItemStack* clicked_item_stack = get_item_stack_on_position((Vector2) { mouse.x, mouse.y });
			if (clicked_item_stack) {
				if (item_stack_held_by_mouse.type == item_type_empty) {
					move_stack(clicked_item_stack, &item_stack_held_by_mouse);
				}
				else if (item_stack_held_by_mouse.type == clicked_item_stack->type) {
					move_stack(&item_stack_held_by_mouse, clicked_item_stack);
				}
				else {
					switch_stacks(clicked_item_stack, &item_stack_held_by_mouse);
				}
			}
		}
		if (mouse.right_button_clicked) {
			ItemStack* clicked_item_stack = get_item_stack_on_position((Vector2) { mouse.x, mouse.y });
			if (clicked_item_stack) {
				if (item_stack_held_by_mouse.type == item_type_empty) {
					move_items(clicked_item_stack, &item_stack_held_by_mouse, clicked_item_stack->amount / 2);
				}
				else {
					move_items(&item_stack_held_by_mouse, clicked_item_stack, 1);
				}
			}
		}
	}
}
