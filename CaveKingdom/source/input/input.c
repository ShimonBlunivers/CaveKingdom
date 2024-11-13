#include <stdbool.h>

#include "input.h"

#include "world/chunk.h"

Mouse mouse = { 0, 0, false, false, false };
Keyboard keyboard = { false, false, false, false };


bool process_input() {
    bool quit = false;

    mouse.left_button_clicked = false;
    mouse.right_button_clicked = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) quit = true;
        else if (event.type == SDL_KEYDOWN) {
            if (SDLK_ESCAPE == event.key.keysym.sym) quit = true;

            else if (SDLK_w == event.key.keysym.sym) keyboard.w_key_pressed = true;
            else if (SDLK_s == event.key.keysym.sym) keyboard.s_key_pressed = true;
            else if (SDLK_a == event.key.keysym.sym) keyboard.a_key_pressed = true;
            else if (SDLK_d == event.key.keysym.sym) keyboard.d_key_pressed = true;
            else if (SDLK_e == event.key.keysym.sym) camera.zoom = SDL_clamp(camera.zoom + .1, camera.min_zoom, camera.max_zoom);
            else if (SDLK_q == event.key.keysym.sym) camera.zoom = SDL_clamp(camera.zoom - .1, camera.min_zoom, camera.max_zoom);
            else if (SDLK_LEFT == event.key.keysym.sym) camera.x -= 10;
            else if (SDLK_RIGHT == event.key.keysym.sym) camera.x += 10;
            else if (SDLK_UP == event.key.keysym.sym) camera.y -= 10;
            else if (SDLK_DOWN == event.key.keysym.sym) camera.y += 10;
            //else if (SDLK_r == event.key.keysym.sym) print_inventory(&main_player->inventory);
        }
        else if (event.type == SDL_KEYUP) {
            if (SDLK_w == event.key.keysym.sym) keyboard.w_key_pressed = false;
            else if (SDLK_s == event.key.keysym.sym) keyboard.s_key_pressed = false;
            else if (SDLK_a == event.key.keysym.sym) keyboard.a_key_pressed = false;
            else if (SDLK_d == event.key.keysym.sym) keyboard.d_key_pressed = false;
        }
        else if (event.type == SDL_MOUSEMOTION) {
            SDL_GetMouseState(&mouse.x, &mouse.y);
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (SDL_BUTTON_LEFT == event.button.button) {
                mouse.left_button_pressed = true;
                mouse.left_button_clicked = true;
            }
            else if (SDL_BUTTON_RIGHT == event.button.button) {
                mouse.right_button_pressed = true;
                mouse.right_button_clicked = true;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            if (SDL_BUTTON_LEFT == event.button.button) {
                mouse.left_button_pressed = false;
            }
            else if (SDL_BUTTON_RIGHT == event.button.button) {
                mouse.right_button_pressed = false;
            }
        }
    }

    return quit;
}

Vector2 from_screen_to_tile_coords(Vector2 screen_coords) {
    Vector2f screen_coords_f = vector2_to_f(screen_coords);

    Vector2f camera_f = vector2_to_f((Vector2) { camera.x, camera.y });

    Vector2f adjusted_coords = vector2f_sum(screen_coords_f, camera_f);

    Vector2f tile_size_f = { TILE_SIZE * camera.zoom, TILE_SIZE * camera.zoom };

    Vector2f tile_coords_f = vector2f_divide(adjusted_coords, tile_size_f);

    return vector2f_to_i(tile_coords_f);
}