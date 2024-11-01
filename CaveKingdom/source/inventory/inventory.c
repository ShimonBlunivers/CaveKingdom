#include <stdio.h>

#include "inventory.h"


Inventory new_inventory(ItemStack content[INVENTORY_SIZE]) {
    Inventory inventory;
    inventory.selected_slot = -1;
    inventory.size = INVENTORY_SIZE;

    for (int i = 0; i < inventory.size; i++) {
        if (content != NULL && content[i].type != item_type_empty) inventory.content[i] = content[i];
        else inventory.content[i] = (ItemStack){ item_type_empty, -1 };
    }

    return inventory;
}

Inventory get_empty_inventory() {
    Inventory inventory;
    inventory.selected_slot = -1;
    inventory.size = INVENTORY_SIZE;

    for (int i = 0; i < inventory.size; i++) inventory.content[i] = (ItemStack){ item_type_empty, -1 };

    return inventory;
}

void select_inventory_slot(Inventory* inventory, int slot_index) {
    if (slot_index < 0 || slot_index > inventory->size) inventory->selected_slot = -1;
    inventory->selected_slot = slot_index;
}

void collect_inventory(Inventory* from, Inventory* to) {
    if (from == NULL || to == NULL) return;
    for (int x = 0; x < from->size; x++) {
        if (from->content[x].type == item_type_empty) continue;
        for (int y = 0; y < to->size; y++) {
            if (to->content[y].type == from->content[x].type) {
                to->content[y].amount += from->content[x].amount;
                from->content[x] = (ItemStack){ item_type_empty, -1 };
                continue;
            }
            if (to->content[y].type == item_type_empty) {
                to->content[y] = from->content[x];
                from->content[x] = (ItemStack){ item_type_empty, -1 };
                continue;
            }
        }
    }
}

bool is_empty_inventory(Inventory inventory) {
    for (int i = 0; i < inventory.size; i++) { // SHOULD IMPLEMENT inventory.used_slots!
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
