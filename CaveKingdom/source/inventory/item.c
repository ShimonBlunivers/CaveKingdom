#include <stdio.h>
#include <SDL.h>

#include "item.h"

void switch_stacks(ItemStack* item_stack_1, ItemStack* item_stack_2) {
	ItemStack temp = *item_stack_1;
	*item_stack_1 = *item_stack_2;
	*item_stack_2 = temp;
}


void move_stack(ItemStack* item_stack_from, ItemStack* item_stack_to) {
	if (item_stack_from->type == item_stack_to->type) {
		int total_amount = item_stack_from->amount + item_stack_to->amount;
		if (total_amount > MAX_STACK_SIZE) {
			item_stack_from->amount = total_amount - MAX_STACK_SIZE;
			item_stack_to->amount = MAX_STACK_SIZE;
		}
		else {
			item_stack_to->amount = total_amount;
			*item_stack_from = (ItemStack){ item_type_empty, -1 };
		}
	}
	else switch_stacks(item_stack_from, item_stack_to);
}

void move_items(ItemStack* item_stack_from, ItemStack* item_stack_to, int amount) {
	amount = SDL_min(amount, item_stack_from->amount);
	if (amount <= 0) return;

	if (item_stack_from->type == item_stack_to->type) {
		int total_amount = amount + item_stack_to->amount;
		if (total_amount > MAX_STACK_SIZE) {
			item_stack_from->amount = total_amount - MAX_STACK_SIZE;
			item_stack_to->amount = MAX_STACK_SIZE;
		}
		else {
			item_stack_to->amount = total_amount;
			item_stack_from->amount = SDL_abs(amount - item_stack_from->amount);
		}
	}
	else if (item_stack_to->type == item_type_empty) {
		item_stack_to->type = item_stack_from->type;
		item_stack_to->amount = amount;

		item_stack_from->amount = SDL_abs(amount - item_stack_from->amount);
	}
	if (item_stack_from->amount <= 0) item_stack_from->type = item_type_empty;
}