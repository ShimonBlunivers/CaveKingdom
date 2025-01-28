#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "core/game.h"
#include "entities/entity.h"
#include "inventory/inventory.h"
#include "audio/audio.h"
#include "world/chunk.h"
#include "world/time.h"
#include "graphics/camera.h"
#include "input/input.h"
#include "graphics/particles.h"
#include "networking/networking.h"

char game_description[32];

int game_status = 1;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Texture* viewport = NULL;
SDL_Texture* gui = NULL;
TTF_Font* font = NULL;

SDL_Surface* cursor_surface = NULL;
SDL_Cursor* cursor = NULL;

typedef enum {
	ui_element_selected_inventory_slot,
	ui_element_inventory_slot,
	ui_element_death_screen,
	ui_element_player_healthbar_outline,
	ui_element_healthbar_outline,
	ui_element_thermometer,
	ui_element_player_inventory_background,

	number_of_ui_elements, // DO NOT USE AS UI ELEMENT!
} UIElement;

SDL_Texture* ui_textures[number_of_ui_elements] = { NULL };
SDL_Texture* entity_textures[number_of_entity_types] = { NULL };
SDL_Texture* item_textures[number_of_item_types] = { NULL };

SDL_Texture* hidden_texture = NULL;
SDL_Texture* shadow_texture = NULL;

void load_textures() {
	// Entity textures
		// Ground
	entity_textures[entity_type_water] = IMG_LoadTexture(renderer, "./assets/textures/tiles/water.png");
	entity_textures[entity_type_dirt] = IMG_LoadTexture(renderer, "./assets/textures/tiles/dirt.png");

	// Surface
	entity_textures[entity_type_player] = IMG_LoadTexture(renderer, "./assets/textures/tiles/player.png");
	entity_textures[entity_type_enemy] = IMG_LoadTexture(renderer, "./assets/textures/tiles/enemy.png");
	entity_textures[entity_type_zombie] = IMG_LoadTexture(renderer, "./assets/textures/tiles/zombie.png");
	entity_textures[entity_type_stone] = IMG_LoadTexture(renderer, "./assets/textures/tiles/stone.png");
	//entity_textures[entity_type_trunk] = IMG_LoadTexture(renderer, "./assets/textures/tiles/trunk.png");

		// Air
	//entity_textures[entity_type_leaves] = IMG_LoadTexture(renderer, "./assets/textures/tiles/leaves.png");
	//SDL_SetTextureAlphaMod(entity_textures[entity_type_leaves], 128);


	// Item textures
	item_textures[item_type_stone] = IMG_LoadTexture(renderer, "./assets/textures/items/stone.png");
	item_textures[item_type_wood] = IMG_LoadTexture(renderer, "./assets/textures/items/wood.png");
	item_textures[item_type_zombie_meat] = IMG_LoadTexture(renderer, "./assets/textures/items/zombie_meat.png");

	// Special
	hidden_texture = IMG_LoadTexture(renderer, "./assets/textures/special/hidden.png");
	shadow_texture = IMG_LoadTexture(renderer, "./assets/textures/special/shadow.png");

	// UI textures
	ui_textures[ui_element_selected_inventory_slot] = IMG_LoadTexture(renderer, "./assets/textures/ui/selected_inventory_slot.png");
	ui_textures[ui_element_inventory_slot] = IMG_LoadTexture(renderer, "./assets/textures/ui/inventory_slot.png");
	ui_textures[ui_element_death_screen] = IMG_LoadTexture(renderer, "./assets/textures/ui/death_screen.png");
	ui_textures[ui_element_player_healthbar_outline] = IMG_LoadTexture(renderer, "./assets/textures/ui/player_healthbar_outline.png");
	ui_textures[ui_element_healthbar_outline] = IMG_LoadTexture(renderer, "./assets/textures/ui/healthbar_outline.png");
	ui_textures[ui_element_thermometer] = IMG_LoadTexture(renderer, "./assets/textures/ui/thermometer.png");
	ui_textures[ui_element_player_inventory_background] = IMG_LoadTexture(renderer, "./assets/textures/ui/player_inventory_background.png");

	// Cursor
	cursor_surface = IMG_Load("./assets/textures/ui/cursor.png");
	cursor = SDL_CreateColorCursor(cursor_surface, 27, 11);

	//Uint8 r, g, b;
	//SDL_GetTextureColorMod(hidden_texture, &r, &g, &b);
	//SDL_SetTextureColorMod(hidden_texture, r / 2, g / 2, b / 2);
}

void unload_textures() {
	for (int i = 0; i < number_of_ui_elements; i++) {
		SDL_DestroyTexture(ui_textures[i]);
	}
	for (int i = 0; i < number_of_entity_types; i++) {
		SDL_DestroyTexture(entity_textures[i]);
	}
	for (int i = 0; i < number_of_item_types; i++) {
		SDL_DestroyTexture(item_textures[i]);
	}

	SDL_DestroyTexture(hidden_texture);
	SDL_DestroyTexture(shadow_texture);

	SDL_FreeSurface(cursor_surface);
	SDL_FreeCursor(cursor);
}

void load_audio() {

}

void unload_audio() {

}

void init_rendering() {
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	viewport = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
	gui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_SetTextureBlendMode(gui, SDL_BLENDMODE_BLEND);
}


Vector2* vision_edge_positions = NULL;

void load_vision_edge_positions(Vector2** edges) {
	*edges = malloc(sizeof(Vector2) * PLAYER_VIEW_DENSITY);
	if (*edges == NULL) return;

	double angle_increment = (2.0f * M_PI) / PLAYER_VIEW_DENSITY;
	double angle = 0;

	for (int i = 0; i < PLAYER_VIEW_DENSITY; i++) {
		int x = (int)round(cos(angle) * TILE_SIZE * PLAYER_VIEW_DISTANCE);
		int y = (int)round(sin(angle) * TILE_SIZE * PLAYER_VIEW_DISTANCE);
		(*edges)[i] = (Vector2){ x, y };
		angle += angle_increment;
	}
}

/**
 * Copy a portion of the texture to the current rendering, with optional
 * rotation and flipping.
 *
 * Copy a portion of the texture to the current rendering target, optionally
 * rotating it by angle around the given center and also flipping it
 * top-bottom and/or left-right.
 *
 * The texture is blended with the destination based on its blend mode set
 * with SDL_SetTextureBlendMode().
 *
 * The texture color is affected based on its color modulation set by
 * SDL_SetTextureColorMod().
 *
 * The texture alpha is affected based on its alpha modulation set by
 * SDL_SetTextureAlphaMod().
 *
 * \param renderer the rendering context
 * \param texture the source texture
 * \param srcrect the source SDL_Rect structure or NULL for the entire texture
 * \param dstrect the destination SDL_Rect structure or NULL for the entire
 *                rendering target
 * \param angle an angle in degrees that indicates the rotation that will be
 *              applied to dstrect, rotating it in a clockwise direction
 * \param center a pointer to a point indicating the point around which
 *               dstrect will be rotated (if NULL, rotation will be done
 *               around `dstrect.w / 2`, `dstrect.h / 2`)
 * \param flip a SDL_RendererFlip value stating which flipping actions should
 *             be performed on the texture
 * \returns 0 on success or a negative error code on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_RenderCopy
 * \sa SDL_SetTextureAlphaMod
 * \sa SDL_SetTextureBlendMode
 * \sa SDL_SetTextureColorMod
 */
void render_copy_ex(SDL_Renderer* renderer,
	SDL_Texture* texture,
	const SDL_Rect* srcrect,
	SDL_Rect* dstrect,
	const double angle,
	const SDL_Point* center,
	const SDL_RendererFlip flip) {


	if (dstrect) {
		SDL_Rect destination_rect = *dstrect;
		destination_rect.x -= camera.x;
		destination_rect.y -= camera.y;
		SDL_RenderCopyEx(renderer, texture, srcrect, &destination_rect, 0, center, flip);
	}
	else SDL_RenderCopyEx(renderer, texture, srcrect, NULL, 0, center, flip);
}

void render_fill_rect(SDL_Renderer* renderer, SDL_Rect* rect) {
	if (rect) {
		SDL_Rect destination_rect = *rect;
		destination_rect.x -= camera.x;
		destination_rect.y -= camera.y;
		SDL_RenderFillRect(renderer, &destination_rect);
	}
	else SDL_RenderFillRect(renderer, NULL);
}

void render_outlined_text(int x, int y, char* text, float text_scale, float outline_offset) {
	int text_width, text_height;

	TTF_SizeUTF8(font, text, &text_width, &text_height);

	SDL_Rect text_rect = {
		.x = (int)round(x),
		.y = (int)round(y),
		.w = (int)round(text_width * text_scale),
		.h = (int)round(text_height * text_scale),
	};

	SDL_Color text_color = { 255, 255, 255, 255 };
	SDL_Color text_outline_color = { 0, 0, 0, 255 };

	SDL_Surface* text_outline_surface = TTF_RenderText_Blended(font, text, text_outline_color);

	SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, text_color);

	SDL_Texture* outline_texture = SDL_CreateTextureFromSurface(renderer, text_outline_surface);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

	SDL_FreeSurface(text_surface);
	SDL_FreeSurface(text_outline_surface);

	SDL_Rect outline_rect = {
		.w = text_rect.w,
		.h = text_rect.h,
	};

	for (int dx = -1; dx <= 1; dx++) { // Outline
		for (int dy = -1; dy <= 1; dy++) {
			if (dx == 0 && dy == 0) continue;
			outline_rect.x = (int)round(text_rect.x + dx * outline_offset);
			outline_rect.y = (int)round(text_rect.y + dy * outline_offset);
			SDL_RenderCopyEx(renderer, outline_texture, NULL, &outline_rect, 0, NULL, false);
		}
	}

	SDL_RenderCopyEx(renderer, text_texture, NULL, &text_rect, 0, NULL, false);

	SDL_DestroyTexture(outline_texture);
	SDL_DestroyTexture(text_texture);
}

void render_item_stack(SDL_Rect item_stack_rect, ItemStack stack) {
	if (stack.type != item_type_empty) {
		int text_padding = 15;
		float text_scale = 0.65f;
		float outline_offset = 2.0;
		int padding = 20;

		SDL_Rect item_rect = { item_stack_rect.x + padding, item_stack_rect.y + padding, item_stack_rect.w - padding * 2, item_stack_rect.h - padding * 2 };

		if (item_textures[stack.type]) {
			SDL_RenderCopyEx(renderer, item_textures[stack.type], NULL, &item_rect, 0, NULL, false);
		}

		char amount[128];
		sprintf_s(amount, sizeof(amount), "%d", stack.amount);

		render_outlined_text(item_stack_rect.x + text_padding, item_stack_rect.y + item_stack_rect.h - 20 - text_padding, amount, text_scale, outline_offset);
	}
}

void draw_world() {

	SDL_SetRenderTarget(renderer, viewport);
	SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
	SDL_RenderClear(renderer);

	{   // Tiles

		Entity* entity_ptr;
		SDL_Rect tile;

		// Checking which tiles are in the player's vision
		for (int i = 0, j, layer; i < PLAYER_VIEW_DENSITY; i++) {
			Vector2 end_position = (Vector2){ vision_edge_positions[i].x - PLAYER_VIEW_DISTANCE / 2, vision_edge_positions[i].y - PLAYER_VIEW_DISTANCE / 2 };
			Vector2f direction_vector = vector2f_divide(vector2_to_f(end_position), (Vector2f) { (float)(TILE_SIZE * PLAYER_VIEW_DISTANCE), (float)(TILE_SIZE * PLAYER_VIEW_DISTANCE) });

			Vector2 current_position;

			for (j = 0; j < PLAYER_VIEW_DISTANCE; j++) {
				current_position = (Vector2){ (int)(direction_vector.x * j), (int)(direction_vector.y * j) };
				entity_ptr = get_entity(current_position.x + main_player->x, current_position.y + main_player->y, height_layer_surface);

				bool transparent = true;
				if (entity_ptr != NULL) {
					for (layer = 0; layer < number_of_height_layers; layer++) {
						Entity* entity_at_layer = get_entity(current_position.x + main_player->x, current_position.y + main_player->y, layer);
						if (entity_at_layer != NULL && entity_at_layer->visibility != NULL) {
							if (entity_at_layer->visibility != NULL) {
								entity_at_layer->visibility->seen = true;
								entity_at_layer->visibility->last_seen = game_tick;
								entity_at_layer->visibility->last_seen_as = entity_at_layer;


								//printf("x: %d ; y: %d ;", current_position.x + main_player->x, current_position.y + main_player->y);
								//printf(" %d\n", entity_at_layer->type);
							}
							if (!entity_at_layer->is_transparent) transparent = false;
						}
					}
				}
				if (entity_ptr == NULL || !transparent) break;
			}
		}


		// Rendering seen tiles
		for (int layer = 0, has_tween, y, x; layer < number_of_height_layers; layer++) {
			for (has_tween = 0; has_tween <= 1; has_tween++) {// Renders tiles with animation on top of others to avoid unexpected layering
				for (int chunk_index = 0; chunk_index < CHUNK_MANAGER.number_of_chunks; chunk_index++) {
					Chunk* chunk = CHUNK_MANAGER.chunks[chunk_index];
					if (chunk->visible) {
						for (y = 0; y < CHUNK_HEIGHT; y++) {
							for (x = 0; x < CHUNK_WIDTH; x++) {
								int x_shifted = x + chunk->x * CHUNK_WIDTH;
								int y_shifted = y + chunk->y * CHUNK_HEIGHT;
								entity_ptr = get_entity(x_shifted, y_shifted, layer);
								if (entity_ptr != NULL) {
									if (entity_ptr->tween != NULL) {
										if (entity_ptr->tween->finish_tick <= (int)graphic_tick) {
											delete_tween(entity_ptr->tween);
											entity_ptr->tween = NULL;
										}
									}
									if (!is_empty_entity_type(entity_ptr->type)) {
										if (has_tween == 0 && entity_ptr->tween == NULL || has_tween == 1 && entity_ptr->tween != NULL) {
											if (entity_ptr->visibility != NULL && entity_ptr->visibility->last_seen == game_tick && entity_ptr->visibility->seen
												&& (entity_textures[entity_ptr->type] != NULL || entity_ptr->type == entity_type_dropped_items)) {
												tile = (SDL_Rect){ TILE_SIZE * x_shifted, TILE_SIZE * y_shifted, TILE_SIZE, TILE_SIZE };
												if (entity_ptr->tween != NULL) {
													Vector2 position = get_current_tween_position(*entity_ptr->tween);

													tile.x = position.x;
													tile.y = position.y;
												}

												if (entity_ptr->type == entity_type_dropped_items) { // Rendering items on the ground
													int item_size = tile.w * 0.25;
													int padding = 10;

													int rendered_items = 0;
													for (int i = 0; i < entity_ptr->inventory->size; i++) {
														ItemStack item_stack = entity_ptr->inventory->content[i];
														if (item_stack.type != item_type_empty) {
															SDL_Rect item_tile = tile;
															item_tile.w = item_size;
															item_tile.h = item_size;

															float relative_x = -tile.w / 2; // Centering
															float relative_y = -tile.h / 2;

															relative_x += (rendered_items * item_size) % (tile.w - padding * 2) + padding;
															relative_y += (rendered_items * item_size) / (tile.w - padding * 2) + padding;

															relative_x *= cos((float)entity_ptr->rotation / 2);
															relative_y *= sin((float)entity_ptr->rotation / 2);

															relative_x += tile.w / 2; // Moving it back
															relative_y += tile.h / 2;

															item_tile.x += relative_x;
															item_tile.y += relative_y;

															SDL_Rect item_shadow_tile = item_tile;
															item_shadow_tile.y += 5; // Shadow offset

															int item_stack_shift = 3;
															for (int rendered_items_in_stack = 0; rendered_items_in_stack < 4 && rendered_items_in_stack < item_stack.amount; rendered_items_in_stack++) {
																item_tile.x += item_stack_shift * rendered_items_in_stack;
																item_shadow_tile.x += item_stack_shift * rendered_items_in_stack;
																render_copy_ex(renderer, shadow_texture, NULL, &item_shadow_tile, (rendered_items_in_stack + entity_ptr->rotation) * 90, NULL, false);

																render_copy_ex(renderer, item_textures[item_stack.type], NULL, &item_tile, (rendered_items_in_stack + entity_ptr->rotation) * 90, NULL, false);
															}

															rendered_items++;
														}
													}
												}
												else {
													render_copy_ex(renderer, entity_textures[entity_ptr->type], NULL, &tile, entity_ptr->rotation * 90, NULL, false);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// Rendering the every unseen as darker
		for (int y = 0, x, layer; y < CHUNK_HEIGHT; y++) {
			for (int chunk_index = 0; chunk_index < CHUNK_MANAGER.number_of_chunks; chunk_index++) {
				Chunk* chunk = CHUNK_MANAGER.chunks[chunk_index];
				if (chunk->visible) {
					for (x = 0; x < CHUNK_WIDTH; x++) {
						int x_shifted = x + chunk->x * CHUNK_WIDTH;
						int y_shifted = y + chunk->y * CHUNK_HEIGHT;
						tile = (SDL_Rect){ TILE_SIZE * x_shifted, TILE_SIZE * y_shifted, TILE_SIZE, TILE_SIZE };
						for (layer = 0; layer < number_of_height_layers; layer++) {
							entity_ptr = get_entity(x_shifted, y_shifted, layer);
							if (entity_ptr != NULL && !is_empty_entity_type(entity_ptr->type) && entity_ptr->visibility != NULL && entity_ptr->visibility->seen && entity_ptr->visibility->last_seen != game_tick && entity_ptr->visibility->last_seen_as != NULL) {
								SDL_Texture* texture = entity_textures[entity_ptr->visibility->last_seen_as->type];
								SDL_SetTextureColorMod(texture, 128, 128, 128);
								render_copy_ex(renderer, texture, NULL, &tile, entity_ptr->visibility->last_seen_as->rotation * 90, NULL, false);
								SDL_SetTextureColorMod(texture, 255, 255, 255);
							}
						}
					}
				}
			}
		}

		for (int y = 0, x, layer; y < CHUNK_HEIGHT; y++) { // Rendering every hidden
			for (int chunk_index = 0; chunk_index < CHUNK_MANAGER.number_of_chunks; chunk_index++) {
				Chunk* chunk = CHUNK_MANAGER.chunks[chunk_index];
				if (chunk->visible) {
					for (x = 0; x < CHUNK_WIDTH; x++) {
						int x_shifted = x + chunk->x * CHUNK_WIDTH;
						int y_shifted = y + chunk->y * CHUNK_HEIGHT;
						bool hidden = true;
						for (layer = 0; layer < number_of_height_layers; layer++) {
							entity_ptr = get_entity(x_shifted, y_shifted, layer);
							if (entity_ptr != NULL && entity_ptr->visibility != NULL && entity_ptr->visibility->seen) {
								hidden = false;
								break;
							}
						}
						if (hidden) {
							float hidden_tile_offset = TILE_SIZE * 0.31;
							tile = (SDL_Rect){ TILE_SIZE * x_shifted - (int)round(hidden_tile_offset), TILE_SIZE * y_shifted - (int)round(hidden_tile_offset), TILE_SIZE + (int)round(hidden_tile_offset * 2), TILE_SIZE + (int)round(hidden_tile_offset * 2) };
							render_copy_ex(renderer, hidden_texture, NULL, &tile, 0, NULL, false);
						}
					}
				}
			}
		}

		{   // Particles
			SDL_Rect particle_rect;
			ParticleListItem* item = PARTICLE_MANAGER.first_particle;
			while (item != NULL) {
				particle_rect = (SDL_Rect){ (int)round(item->particle.x), (int)round(item->particle.y), item->particle.size, item->particle.size };
				SDL_SetRenderDrawColor(renderer, item->particle.color.r, item->particle.color.g, item->particle.color.b, item->particle.color.a);
				render_fill_rect(renderer, &particle_rect);
				item = item->next_list_item;
			}
		}

		{   // Healthbars
			Entity* entity_ptr;
			SDL_Rect tile;
			int max_healthbar_width = TILE_SIZE;
			int max_healthbar_height = 16;
			for (int y = 0, x, layer; y < CHUNK_HEIGHT; y++) {
				for (int chunk_index = 0; chunk_index < CHUNK_MANAGER.number_of_chunks; chunk_index++) {
					Chunk* chunk = CHUNK_MANAGER.chunks[chunk_index];
					if (chunk->visible) {
						for (x = 0; x < CHUNK_WIDTH; x++) {
							int x_shifted = x + chunk->x * CHUNK_WIDTH;
							int y_shifted = y + chunk->y * CHUNK_HEIGHT;
							for (layer = 0; layer < number_of_height_layers; layer++) {
								entity_ptr = get_entity(x_shifted, y_shifted, layer);
								if (entity_ptr != NULL && thermal_vision) { // !!!!!!!!!
									tile = (SDL_Rect){ TILE_SIZE * x_shifted, TILE_SIZE * y_shifted, TILE_SIZE, TILE_SIZE };
									SDL_SetTextureColorMod(entity_textures[entity_ptr->type], (int)(255 * (entity_ptr->thermal.temperature - 250) / entity_ptr->thermal.max_temperature) % 255, 0, 0);
									render_copy_ex(renderer, entity_textures[entity_ptr->type], NULL, &tile, entity_ptr->rotation * 90, NULL, false);
									SDL_SetTextureColorMod(entity_textures[entity_ptr->type], 255, 255, 255);
								}// !!!!!!!!!!!!
								if (entity_ptr != NULL && entity_ptr->visibility != NULL && !is_empty_entity_type(entity_ptr->type) && entity_ptr->visibility->last_seen == game_tick && entity_ptr->visibility->seen) {
									if (entity_ptr->type == entity_type_player) continue;
									if (entity_ptr->health->max > 0 && entity_ptr->health->max != entity_ptr->health->current) {
										tile = (SDL_Rect){ TILE_SIZE * x_shifted, TILE_SIZE * y_shifted, TILE_SIZE, TILE_SIZE };
										if (entity_ptr->tween != NULL) {
											Vector2 position = get_current_tween_position(*entity_ptr->tween);
											tile.x = position.x;
											tile.y = position.y;
										}
										int tile_x = tile.x - max_healthbar_width / 2 + TILE_SIZE / 2;
										int tile_y = tile.y;

										Vector2 padding = { 2, 2 };
										SDL_Rect outline_rect = { tile_x, tile_y, max_healthbar_width, max_healthbar_height };
										SDL_Rect background_rect = { outline_rect.x + padding.x, outline_rect.y + padding.y, outline_rect.w - padding.x * 2, outline_rect.h - padding.y * 2 };
										SDL_Rect health_rect = { background_rect.x, background_rect.y, (int)(background_rect.w * ((float)entity_ptr->health->current / entity_ptr->health->max)), background_rect.h };

										SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
										render_fill_rect(renderer, &background_rect);
										SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
										render_fill_rect(renderer, &health_rect);
										render_copy_ex(renderer, ui_textures[ui_element_healthbar_outline], NULL, &outline_rect, 0, NULL, false);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	{   // UI

		SDL_SetRenderTarget(renderer, gui);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);


		if (main_player != NULL && main_player_alive) {

			if (!inventory_opened) {

				{ // Temperature

					SDL_Rect thermometer_rect = { 4 * UI_RATIO, 82 * UI_RATIO, 39 * UI_RATIO, 156 * UI_RATIO };

					SDL_Rect background_rect = { thermometer_rect.x + 14 * UI_RATIO - 2, thermometer_rect.y + 13 * UI_RATIO - 2, thermometer_rect.w / 3, thermometer_rect.h - 27 * UI_RATIO };

					double percentage = ((main_player->thermal.temperature - 273.15 - 36) / (100 - 36)) * 50 + 50;

					int difference = percentage * background_rect.h / 100;

					difference = SDL_clamp(difference, 1, background_rect.h - 1);

					SDL_Rect temperature_rect = { background_rect.x, background_rect.y + background_rect.h - difference, background_rect.w, difference };

					SDL_SetRenderDrawColor(renderer, 180, 200, 220, 110);
					SDL_RenderFillRect(renderer, &background_rect);
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
					SDL_RenderFillRect(renderer, &temperature_rect);
					SDL_RenderCopyEx(renderer, ui_textures[ui_element_thermometer], NULL, &thermometer_rect, 0, NULL, false);
				}

				{ // Player healthbar
					int healthbar_width = 312 * UI_RATIO;
					int healthbar_height = 20 * UI_RATIO;

					SDL_Rect healthbar_rect = { (SCREEN_WIDTH - healthbar_width) / 2,  SCREEN_HEIGHT - healthbar_height - 91, healthbar_width, healthbar_height };
					Vector2 padding = { 2 * UI_RATIO, 2 * UI_RATIO };
					SDL_Rect background_rect = { healthbar_rect.x + padding.x, healthbar_rect.y + padding.y, healthbar_rect.w - padding.x * 2, healthbar_rect.h - padding.y * 2 };
					SDL_Rect health_rect = { background_rect.x, background_rect.y, (int)(background_rect.w * ((float)main_player->health->current / main_player->health->max)), background_rect.h };

					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_RenderFillRect(renderer, &background_rect);
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
					SDL_RenderFillRect(renderer, &health_rect);
					SDL_RenderCopyEx(renderer, ui_textures[ui_element_player_healthbar_outline], NULL, &healthbar_rect, 0, NULL, false);

				}
			}

			{ // Inventory
				Inventory* inventory = main_player->inventory;
				if (inventory) {
					if (inventory_opened) {
						SDL_RenderCopyEx(renderer, ui_textures[ui_element_player_inventory_background], NULL, &player_inventory_rect, 0, NULL, false);

						for (int i = 0; i < inventory->size; i++) {
							SDL_Rect slot_rect = get_player_slot_rect(inventory, i);
							//SDL_RenderCopyEx(renderer, ui_textures[ui_element_inventory_slot], NULL, &slot_rect, 0, NULL, false);

							if (inventory->selected_slot == i) SDL_RenderCopyEx(renderer, ui_textures[ui_element_selected_inventory_slot], NULL, &slot_rect, 0, NULL, false);

							render_item_stack(slot_rect, inventory->content[i]);
						}
					}
					else {
						for (int i = 0; i < INVENTORY_HOTBAR_SLOT_COUNT; i++) {
							if (i >= inventory->size) continue;

							SDL_Rect slot_rect = get_player_slot_rect(inventory, i);

							SDL_RenderCopyEx(renderer, ui_textures[ui_element_inventory_slot], NULL, &slot_rect, 0, NULL, false);

							if (inventory->selected_slot == i) SDL_RenderCopyEx(renderer, ui_textures[ui_element_selected_inventory_slot], NULL, &slot_rect, 0, NULL, false);

							render_item_stack(slot_rect, inventory->content[i]);
						}
					}
				}

				render_item_stack((SDL_Rect) { mouse.x - 40, mouse.y - 16, 80, 80 }, item_stack_held_by_mouse);

			}
		}

		// Death screen
		if (!main_player_alive) {
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
			SDL_RenderFillRect(renderer, NULL);
			render_copy_ex(renderer, ui_textures[ui_element_death_screen], NULL, NULL, 0, NULL, false);
		}

		render_outlined_text(10, 10, game_description, 0.5f, 1.0);
	}

	SDL_SetRenderTarget(renderer, NULL);
	SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
	SDL_RenderClear(renderer);

	SDL_Rect destination_rect = { 0, 0, (int)(SCREEN_WIDTH * camera.zoom), (int)(SCREEN_HEIGHT * camera.zoom) };

	SDL_RenderCopy(renderer, viewport, NULL, &destination_rect);
	SDL_RenderCopy(renderer, gui, NULL, NULL);
	SDL_RenderPresent(renderer);
}



int main(int argc, char* argv[]) {
	srand((int)(time(NULL)));

	init_chunk_manager();


	//  Testing setup
	{
		Vector2 pos = find_empty_tile();
		force_spawn_entity(new_entity(entity_type_enemy, pos.x, pos.y));
		pos = find_empty_tile();
		force_spawn_entity(new_entity(entity_type_zombie, pos.x, pos.y));
	}


	//

	spawn_player();

	// Networking
	setup_server();
	//


	// Setup game description
	snprintf(game_description, sizeof(game_description), "CaveKingdom v%s", GAME_VERSION);


	if (TTF_Init() == -1)
	{
		fprintf(stderr, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		return 1;
	}
	else if (!main_player) {
		fprintf(stderr, "The main player is NULL!\n");
		return 1;
	}
	else if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}
	else if (!IMG_Init(IMG_INIT_PNG)) {
		fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return 1;
	}
	else {
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 16);

		window = SDL_CreateWindow("CaveKingdom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);

		font = TTF_OpenFont("./assets/fonts/roboto.ttf", 32);

		if (font == NULL)
		{
			fprintf(stderr, "Failed to load roboto font! SDL_ttf Error: %s\n", TTF_GetError());
			return 1;
		}
		else if (window == NULL) {
			fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return 1;
		}
		else {
			init_rendering();
			init_input();
			load_textures();
			load_audio();

			SDL_SetCursor(cursor);

			load_vision_edge_positions(&vision_edge_positions);

			bool quit = false;

			graphic_tick = SDL_GetTicks();

			previous_counter = SDL_GetPerformanceCounter();

			update_time();
			Uint32 update_delay = 250; // 250
			while (!quit) {
				last_updated_tick = SDL_GetTicks();

				main_player_updated = false;

				update_entities();
				update_server();

				while (!quit && graphic_tick < last_updated_tick + update_delay) {
					quit = process_input();
					update_player_inventory();

					if (!main_player_updated && main_player_alive) {
						main_player_updated = update_player();
					}

					update_camera();
					update_particles();
					draw_world();

					update_time();
				}

				game_tick++;
			}
		}
	}

	unload_textures();
	unload_audio();

	TTF_CloseFont(font);

	SDL_DestroyWindow(window);

	IMG_Quit();
	TTF_Quit();
	SDL_Quit();

	free_world();
	free(vision_edge_positions);

	close_server();

	return 0;
}