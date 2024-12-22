#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#include "entities/entity.h"

#include "core/game.h"

#include "input/input.h"
#include "world/chunk.h"

#include "world/time.h"
#include "graphics/particles.h"

Entity* main_player = NULL;
bool main_player_alive = true;
int number_of_entities = 0;

bool main_player_updated = false;

// List of empty types for every layer, ascending
const EntityType empty_entity_types[number_of_height_layers] = {
	entity_type_ground_empty,
	entity_type_surface_empty,
	entity_type_air_empty,
};

//Entity entity_list[CHUNK_WIDTH * CHUNK_HEIGHT * number_of_height_layers];
//Entity* entity_position_grid[CHUNK_WIDTH * CHUNK_HEIGHT][number_of_height_layers]; // For quicker access through coordinates

bool is_empty_entity_type(EntityType entity_type) {
	for (int i = 0; i < number_of_height_layers; i++) if (entity_type == empty_entity_types[i]) return true;
	return false;
}

Entity new_entity(EntityType type, int x, int y) {

	HeightLayer height_layer;

	Entity new_entity = {
		.id = number_of_entities++,
		.type = type,
		.color = (SDL_Color){ 128, 128, 128, 255 },
		.height_layer = height_layer_ground,
		.x = x,
		.y = y,
		.is_obstacle = false,
		.is_transparent = -1,
		.rotation = 0,
		.visibility = malloc(sizeof(Visibility)),
		.health = malloc(sizeof(Health)),
		.thermal = (Thermal) {
			.temperature = 300,
			.conductivity = 0.01,
			.max_temperature = 400, // 10000
			.min_temperature = 1, // 0.1
			.generated_heat_per_tick = 0,
		},
		.hunger = NULL,
		.combat = NULL,
		.brain = NULL,
		.inventory = NULL,

		.tween = NULL,
	};

	bool randomize_rotation = false;

	if (new_entity.visibility != NULL) *new_entity.visibility = (Visibility){ false, false, -1 };
	else printf("Error: new_entity.visibility was NULL when creating new entity.\n");
	if (new_entity.health != NULL) *new_entity.health = (Health){ -1, -1 };
	else printf("Error: new_entity.health was NULL when creating new entity.\n");

	bool entity_struct_initiated = false;
	new_entity.height_layer = height_layer_ground;

	switch (type) {
	case entity_type_water:
		entity_struct_initiated = true;

		new_entity.color = (SDL_Color){ 144, 203, 239, 255 };

		new_entity.is_obstacle = true;
		break;

	case entity_type_dirt:
		entity_struct_initiated = true;
		randomize_rotation = true;

		new_entity.color = (SDL_Color){ 89, 60, 44, 255 };
		break;

	case entity_type_ground_empty:
		entity_struct_initiated = true;
		break;
	}

	if (!entity_struct_initiated) new_entity.height_layer = height_layer_surface;

	switch (type) {
	case entity_type_dropped_items:
		entity_struct_initiated = true;
		randomize_rotation = true;
		new_entity.inventory = new_inventory(INVENTORY_SIZE);
		break;

	case entity_type_player:
		entity_struct_initiated = true;

		new_entity.color = (SDL_Color){ 217, 149, 149, 255 };

		new_entity.combat = (Combat*)malloc(sizeof(Combat));
		new_entity.hunger = (Hunger*)malloc(sizeof(Hunger));

		if (new_entity.combat != NULL) *new_entity.combat = (Combat){ 1, 0 };
		if (new_entity.hunger != NULL) *new_entity.hunger = (Hunger){ 100 };

		new_entity.inventory = new_inventory(INVENTORY_SIZE);
		new_entity.health->max = 10;
		new_entity.is_obstacle = true;
		new_entity.is_transparent = true;
		break;

	case entity_type_enemy:
		entity_struct_initiated = true;

		new_entity.color = (SDL_Color){ 217, 149, 149, 255 };

		new_entity.brain = (Brain*)malloc(sizeof(Brain));
		new_entity.combat = (Combat*)malloc(sizeof(Combat));
		new_entity.hunger = (Hunger*)malloc(sizeof(Hunger));

		if (new_entity.brain != NULL) *new_entity.brain = (Brain){ true, 50, (Vector2f) { 0., 0. } };
		if (new_entity.combat != NULL) *new_entity.combat = (Combat){ 1, 0 };
		if (new_entity.hunger != NULL) *new_entity.hunger = (Hunger){ 100 };

		new_entity.inventory = new_inventory(INVENTORY_SIZE);
		new_entity.health->max = 10;
		new_entity.is_obstacle = true;
		new_entity.is_transparent = true;
		break;

	case entity_type_zombie:
		entity_struct_initiated = true;

		new_entity.color = (SDL_Color){ 137, 172, 140, 255 };

		new_entity.brain = (Brain*)malloc(sizeof(Brain));
		new_entity.combat = (Combat*)malloc(sizeof(Combat));
		new_entity.hunger = (Hunger*)malloc(sizeof(Hunger));

		if (new_entity.brain != NULL) *new_entity.brain = (Brain){ true, 50, (Vector2f) { 0., 0. } };
		if (new_entity.combat != NULL) *new_entity.combat = (Combat){ 1, 0 };
		if (new_entity.hunger != NULL) *new_entity.hunger = (Hunger){ 100 };

		new_entity.inventory = new_inventory(INVENTORY_SIZE);
		add_to_inventory(new_entity.inventory, (ItemStack) { item_type_zombie_meat, 2 });

		new_entity.health->max = 10;
		new_entity.is_obstacle = true;
		new_entity.is_transparent = true;
		break;

	case entity_type_wall:
		entity_struct_initiated = true;

		new_entity.is_obstacle = true;
		break;

	case entity_type_stone:
		entity_struct_initiated = true;

		new_entity.color = (SDL_Color){ 97, 97, 97, 255 };

		new_entity.inventory = new_inventory(1);
		add_to_inventory(new_entity.inventory, (ItemStack) { item_type_stone, 3 });

		new_entity.health->max = 5;
		new_entity.is_obstacle = true;

		break;

	case entity_type_surface_empty:
		entity_struct_initiated = true;
		break;
	}

	if (!entity_struct_initiated) new_entity.height_layer = height_layer_air;
	switch (type) {

	case entity_type_air_empty:
		entity_struct_initiated = true;
		break;
	}

	if (is_empty_entity_type(new_entity.type) || new_entity.type == entity_type_dropped_items) {
		new_entity.thermal.conductivity = 0.1;
	}


	if (randomize_rotation) new_entity.rotation = rand() % 5;
	if (new_entity.health->current == -1) new_entity.health->current = new_entity.health->max;
	if (new_entity.is_transparent == -1) new_entity.is_transparent = !new_entity.is_obstacle;

	return new_entity;
}

void free_entity(Entity* entity) {
	free(entity->visibility);
	free(entity->combat);
	free(entity->hunger);
	free(entity->health);
	free(entity->brain);
	free_inventory(entity->inventory);
	delete_tween(entity->tween);
}

void destroy_entity(Entity* entity) {
	if (entity->id == main_player->id) main_player_alive = false;

	Entity created_entity;

	if (entity->inventory != NULL && entity->type != entity_type_dropped_items) {
		created_entity = new_entity(entity_type_dropped_items, entity->x, entity->y);
		collect_inventory(entity->inventory, created_entity.inventory);
	}
	else created_entity = new_entity(empty_entity_types[entity->height_layer], entity->x, entity->y);

	created_entity.thermal.temperature = entity->thermal.temperature;

	free_entity(entity);

	for (int layer = 0; layer < number_of_height_layers; layer++) {
		if (layer == created_entity.height_layer) continue;
		Entity* e = get_entity(created_entity.x, created_entity.y, layer);
		if (e->visibility == NULL) continue;
		e->visibility->seen = false;
		e->visibility->last_seen_as = NULL;
	}

	Chunk* chunk = get_chunk_from_global_position(entity->x, entity->y);

	int x = entity->x;
	int y = entity->y;

	if (x < 0) x += CHUNK_WIDTH;
	if (y < 0) y += CHUNK_HEIGHT;

	*chunk->entity_position_grid[(y % CHUNK_HEIGHT) * CHUNK_WIDTH + x % CHUNK_WIDTH][created_entity.height_layer] = created_entity;
}

Entity* get_entity(int x, int y, HeightLayer layer) {
	if (layer < 0 || layer >= number_of_height_layers) return NULL;

	Chunk* chunk = get_chunk_from_global_position(x, y);

	if (chunk == NULL) return NULL;

	return get_entity_from_chunk(chunk, x, y, layer);
}

bool set_entity(int x, int y, Entity* entity) {
	Chunk* chunk = get_chunk_from_global_position(x, y);
	if (chunk == NULL) return false;

	if (x < 0) x += CHUNK_WIDTH;
	if (y < 0) y += CHUNK_HEIGHT;

	chunk->entity_position_grid[(y % CHUNK_HEIGHT) * CHUNK_WIDTH + x % CHUNK_WIDTH][entity->height_layer] = entity;
	return true;
}

bool spawn_entity(Entity entity) {
	Entity* entity_pointer = get_entity(entity.x, entity.y, entity.height_layer);
	if (!is_empty_entity_type(entity_pointer->type)) return false;
	*entity_pointer = entity;
	if (entity.type == entity_type_player) {
		main_player = entity_pointer;
	}
	return true;
}

void force_spawn_entity(Entity entity) {
	Entity* old_entity = get_entity(entity.x, entity.y, entity.height_layer);
	free_entity(old_entity);
	*old_entity = entity;
	if (entity.type == entity_type_player) {
		main_player = old_entity;
	}
}

bool hit_entity(Entity* hitter, Entity* target) {
	if (target == NULL || hitter->combat == NULL) return false;
	if (hitter->combat->damage <= 0 || target->health->max < 0) return false;

	int damage = hitter->combat->damage - ((target->combat != NULL) ? target->combat->armor : 0);
	target->health->current -= SDL_max(damage, 0);

	if (target->visibility->seen && game_tick - target->visibility->last_seen <= 2) for (int i = 0; i < 5; i++)
		new_particle((target->x + 0.25 + ((float)(rand() % 6)) / 10) * TILE_SIZE, (target->y + 0.25 + ((float)(rand() % 6)) / 10) * TILE_SIZE, target->color);

	if (target->health->current <= 0) {
		//if (target->inventory != NULL && hitter->inventory != NULL) collect_inventory(target->inventory, hitter->inventory);
		destroy_entity(target);
	}
	return true;
}

void switch_entities(Entity* entity, Entity* neighbour_entity) {
	int x1 = entity->x;
	int y1 = entity->y;

	int x2 = neighbour_entity->x;
	int y2 = neighbour_entity->y;

	{ // Tween
		if (neighbour_entity->type == entity_type_dropped_items && entity->inventory != NULL) {
			collect_inventory(neighbour_entity->inventory, entity->inventory);
			destroy_entity(neighbour_entity);
		}

		if (entity->tween == NULL) {
			entity->tween = new_tween(x1 * TILE_SIZE, y1 * TILE_SIZE, x2 * TILE_SIZE, y2 * TILE_SIZE, graphic_tick + move_tile_tween_duration);

			//printf("2 x: %d ; y: %d\n", x2 * TILE_SIZE - x1 * TILE_SIZE, y2 * TILE_SIZE - y1 * TILE_SIZE);
		}
		else {
			*entity->tween = change_finish_tween(*entity->tween, (Vector2) { x2* TILE_SIZE, y2* TILE_SIZE });
		}

		if (neighbour_entity->tween == NULL) {
			neighbour_entity->tween = new_tween(x2 * TILE_SIZE, y2 * TILE_SIZE, x1 * TILE_SIZE, y1 * TILE_SIZE, graphic_tick + move_tile_tween_duration);
		}
		else {
			*neighbour_entity->tween = change_finish_tween(*neighbour_entity->tween, (Vector2) { x1* TILE_SIZE, y1* TILE_SIZE });
		}
	}

	neighbour_entity->x = entity->x;
	neighbour_entity->y = entity->y;

	entity->x = x2;
	entity->y = y2;

	Visibility* temp_visibility = entity->visibility;
	entity->visibility = neighbour_entity->visibility;
	neighbour_entity->visibility = temp_visibility;

	Entity* temp_entity = entity;
	set_entity(x1, y1, neighbour_entity);
	set_entity(x2, y2, temp_entity);
}

bool is_tile_obstacle(int x, int y) {
	for (int layer = 0; layer < number_of_height_layers; layer++)
		if (get_entity(x, y, layer)->is_obstacle) return true;
	return false;
}

Vector2 find_empty_tile() {
	int start_x = rand() % CHUNK_WIDTH;
	int start_y = rand() % CHUNK_HEIGHT;

	Vector2 spare_position = { -1, -1 };

	int ignore_positions = 5;

	int x, y;
	for (int _y = 0; _y < CHUNK_HEIGHT; _y++) {
		for (int _x = 0; _x < CHUNK_WIDTH; _x++) {
			x = (_x + start_x) % CHUNK_WIDTH;
			y = (_y + start_y) % CHUNK_HEIGHT;
			if (!is_tile_obstacle(x, y)) {
				if (ignore_positions-- == 0) return (Vector2) { x, y };
				spare_position = (Vector2){ x, y };
			}
		}
	}

	return spare_position;
}

void spawn_player() {
	Vector2 spawn_position = find_empty_tile();
	force_spawn_entity(new_entity(entity_type_player, spawn_position.x, spawn_position.y));
}

bool move_entity(Entity* entity, int x, int y) {
	Entity* neighbour = get_entity(entity->x + x, entity->y + y, entity->height_layer);

	if (neighbour == NULL) return false;

	bool collided = is_tile_obstacle(entity->x + x, entity->y + y);
	if (collided) return false;



	switch_entities(entity, neighbour);

	return true;
}


int random_direction() {
	return 1 - rand() % 3;
}

float get_movement_randomisation() {
	return .9 + ((rand() % 3)) / 10.;
}

void add_heat_entity(Entity* entity, float heat) {
	heat *= entity->thermal.conductivity;
	if (entity->thermal.temperature + heat >= entity->thermal.max_temperature) entity->thermal.temperature = entity->thermal.max_temperature;
	else if (entity->thermal.temperature + heat <= entity->thermal.min_temperature) entity->thermal.temperature = entity->thermal.min_temperature;
	else entity->thermal.temperature += heat;
}

void update_temperature_entity(Entity* entity) {
	for (int j = 0; j < 6; j++) {
		int x = 0;
		int y = 0;
		int layer = 0;

		if (j < 2) x += j % 2 == 0 ? -1 : 1;
		else if (j > 3) y += j % 2 == 0 ? -1 : 1;
		else layer += j % 2 == 0 ? -1 : 1;

		Entity* neighbour = get_entity(entity->x + x, entity->y + y, entity->height_layer + layer);
		if (neighbour == NULL) continue;

		float delta_temp = entity->thermal.temperature - neighbour->thermal.temperature;
		if (round(delta_temp) != 0) {
			float heat_transfer = delta_temp / 2;

			add_heat_entity(entity, -heat_transfer);
			add_heat_entity(neighbour, +heat_transfer);

			//printf("Heat transfer: %f\n", heat_transfer);
			//printf("Neighbour temperature: %f\n", neighbour->thermal.temperature);
		}
	}
	entity->thermal.temperature += entity->thermal.generated_heat_per_tick;
}
void update_entities() {
	for (int chunk_index = 0; chunk_index < CHUNK_MANAGER.number_of_chunks; chunk_index++) {
		Chunk* chunk = CHUNK_MANAGER.chunks[chunk_index];
		for (int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT * number_of_height_layers; i++) {
			Entity* entity = &chunk->entity_list[i];
			update_temperature_entity(entity);
			if (entity->brain != NULL && entity->brain->active) {
				for (int j = 0; j < 4; j++) {
					int x = j < 2 ? -1 + (j % 2) * 2 : 0;
					int y = j >= 2 ? -1 + ((j + 2) % 2) * 2 : 0;
					Entity* neighbour = get_entity(entity->x + x, entity->y + y, entity->height_layer);
					if (neighbour == NULL) entity->brain->desired_direction = vector2f_sum(entity->brain->desired_direction, (Vector2f) { -x * .2, -y * .2 });
					else if (neighbour->health->current > 0) {
						entity->brain->desired_direction = vector2f_sum(entity->brain->desired_direction, (Vector2f) { x * .5, y * .5 });
						hit_entity(entity, neighbour);
					}
					else if (neighbour->is_obstacle || get_entity(entity->x + x, entity->y + y, entity->height_layer - 1)->is_obstacle)
						entity->brain->desired_direction = vector2f_sum(entity->brain->desired_direction, (Vector2f) { -x * .2, -y * .2 });
				}

				entity->brain->desired_direction = vector2f_sum(entity->brain->desired_direction, (Vector2f) { -.1 + ((rand() % 3)) / 10., -.1 + ((rand() % 3)) / 10. });

				entity->brain->desired_direction.x = SDL_clamp(entity->brain->desired_direction.x, -1., 1.);
				entity->brain->desired_direction.y = SDL_clamp(entity->brain->desired_direction.y, -1., 1.);

				if (entity->brain->desired_direction.x > 0.5 * get_movement_randomisation())
					move_entity(entity, 1, 0);
				else if (entity->brain->desired_direction.x < -0.5)
					move_entity(entity, -1, 0);
				if (entity->brain->desired_direction.y > 0.5)
					move_entity(entity, 0, 1);
				else if (entity->brain->desired_direction.y < -0.5 * get_movement_randomisation())
					move_entity(entity, 0, -1);
			}

			switch (entity->type) {
				break;
			case entity_type_zombie:
			case entity_type_enemy:
				break;
			}
		}
	}
}

bool update_player() {
	//Vector2 mouse_position = from_screen_to_tile_coords((Vector2) { mouse.x, mouse.y });
	//Entity* entity_hovered = get_entity(mouse_position.x, mouse_position.y, height_layer_surface);
	//printf("Temperature: %f\n", entity_hovered->thermal.temperature);

	bool updated = false;

	if (mouse.left_button_pressed) {
		Vector2 clicked_tile_position = from_screen_to_tile_coords((Vector2) { mouse.x, mouse.y });
		Vector2 distance = vector2_subtract(clicked_tile_position, (Vector2) { main_player->x, main_player->y });

		if (abs(distance.x) <= 1 && abs(distance.y) <= 1) {
			Entity* entity_clicked = get_entity(clicked_tile_position.x, clicked_tile_position.y, height_layer_surface);

			if (entity_clicked != NULL && entity_clicked->type != entity_type_player) {
				updated |= hit_entity(main_player, entity_clicked);

				entity_clicked->thermal.temperature = 1000; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
		}
	}

	int player_movement_x = 0;
	int player_movement_y = 0;

	if (should_player_move_with_key(&keyboard[key_w])) player_movement_y--;
	if (should_player_move_with_key(&keyboard[key_s])) player_movement_y++;
	if (should_player_move_with_key(&keyboard[key_a])) player_movement_x--;
	if (should_player_move_with_key(&keyboard[key_d])) player_movement_x++;

	bool moved_x = false;
	bool moved_y = false;
	if (player_movement_x) {
		moved_x = move_entity(main_player, player_movement_x, 0);
		updated |= moved_x;
	}
	if (player_movement_y) {
		moved_y = move_entity(main_player, 0, player_movement_y);
		if (player_movement_x && !moved_x) {
			move_entity(main_player, player_movement_x, 0);
		}
		updated |= moved_x | moved_y;
	}
	//printf("player_movement_x: %d ; player_movement_y: %d ; moved_x: %d ; moved_y: %d\n", player_movement_x, player_movement_y, moved_x, moved_y);

	return updated;
}