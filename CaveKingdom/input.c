#include <stdbool.h>

#include "input.h"

Mouse mouse = { 0, 0, false, false, false };
Keyboard keyboard = { false, false, false, false };


bool process_input() {
    bool quit = false;

    SDL_Event event;

    mouse.left_button_clicked = false;

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