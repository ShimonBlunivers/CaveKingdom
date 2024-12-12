#include "input.h"
#include "core/game.h"
#include "world/chunk.h"
#include "world/time.h"
#include "entities/entity.h"

Mouse mouse = { 0, 0, false, false, false };
Key keyboard[number_of_keys];

void init_input() {
    for (KeyCode key_code = 0; key_code < number_of_keys; key_code++) {
        keyboard[key_code] = (Key){
            .pressed = false,
            .tick_pressed = 0,
            .sdl_key_code = get_sdl_key_code(key_code),
        };
    }
}

static SDL_KeyCode get_sdl_key_code(KeyCode code) {
    switch (code)
    {
        case key_w:
            return SDLK_w;
        case key_s:
            return SDLK_s;
        case key_a:
            return SDLK_a;
        case key_d:
            return SDLK_d;
        case key_f:
            return SDLK_f;
        case key_1:
            return SDLK_1;
        case key_2:
            return SDLK_2;
        case key_3:
            return SDLK_3;
        case key_4:
            return SDLK_4;
        case key_5:
            return SDLK_5;
        case key_6:
            return SDLK_6;
        case key_7:
            return SDLK_7;
        case key_8:
            return SDLK_8;
        case key_9:
            return SDLK_9;
        case key_0:
            return SDLK_0;
    default:
        return 0;
    }
}

static KeyCode get_key_code(SDL_KeyCode sdl_key_code) {
    for (KeyCode key_code = 0; key_code < number_of_keys; key_code++) {
        if (keyboard[key_code].sdl_key_code == sdl_key_code) return key_code;
    }
    return number_of_keys;
}

static void key_press(KeyCode key) {
    if (!keyboard[key].pressed) keyboard[key].tick_pressed = graphic_tick;
    keyboard[key].pressed = true;
}

bool key_tapped(Key key) {
    return key.tick_pressed == graphic_tick;
}

bool process_input() {
    bool quit = false;

    mouse.left_button_clicked = false;
    mouse.right_button_clicked = false;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) quit = true;
        else if (event.type == SDL_KEYDOWN) {
            KeyCode key_code = get_key_code(event.key.keysym.sym);
            if (key_code != number_of_keys) key_press(key_code);

            else if (SDLK_ESCAPE == event.key.keysym.sym) quit = true;
            else if (SDLK_e == event.key.keysym.sym) camera.zoom = SDL_clamp(camera.zoom + .1, camera.min_zoom, camera.max_zoom);
            else if (SDLK_q == event.key.keysym.sym) camera.zoom = SDL_clamp(camera.zoom - .1, camera.min_zoom, camera.max_zoom);
            else if (SDLK_LEFT == event.key.keysym.sym) camera.x -= 10;
            else if (SDLK_RIGHT == event.key.keysym.sym) camera.x += 10;
            else if (SDLK_UP == event.key.keysym.sym) camera.y -= 10;
            else if (SDLK_DOWN == event.key.keysym.sym) camera.y += 10;
            //else if (SDLK_r == event.key.keysym.sym) print_inventory(&main_player->inventory);
            
        }
        else if (event.type == SDL_KEYUP) {
            KeyCode key_code = get_key_code(event.key.keysym.sym);
            if (key_code != number_of_keys) keyboard[key_code].pressed = false;
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
            if (SDL_BUTTON_LEFT == event.button.button) mouse.left_button_pressed = false;
            else if (SDL_BUTTON_RIGHT == event.button.button) mouse.right_button_pressed = false;
        }
    }

    if (key_tapped(keyboard[key_f])) thermal_vision = !thermal_vision;

    return quit;
}


Vector2 from_screen_to_tile_coords(Vector2 screen_coords) {
    
    Vector2f screen_coords_f = vector2_to_f(screen_coords);

    screen_coords_f = vector2f_divide(screen_coords_f, (Vector2f){camera.zoom, camera.zoom});

    Vector2f camera_f = vector2_to_f((Vector2) { camera.x, camera.y});

    Vector2f adjusted_coords = vector2f_sum(screen_coords_f, camera_f);

    Vector2f tile_size_f = { (float)(TILE_SIZE), (float)(TILE_SIZE) };

    Vector2f tile_coords_f = vector2f_divide(adjusted_coords, tile_size_f);

    return vector2f_to_i(tile_coords_f);
}

bool should_player_move_with_key(Key key) {
    if (!key.pressed) return false;

    int double_move_prevention = 150;
    return key.tick_pressed == graphic_tick || graphic_tick - key.tick_pressed > double_move_prevention;
}       