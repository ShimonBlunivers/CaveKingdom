#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "entity.h"
#include "inventory.h"
#include "chunk.h"
#include "camera.h"
#include "renderer.h"


int game_status = 1;

int main(void) {
    
    reset_grids();
    create_edge_walls();

    //  Testing setup
    
    spawn_entity(new_entity(entity_type_player, 11, 11));

    //spawn_entity(new_entity(entity_type_wall, 3, 4));
    spawn_entity(new_entity(entity_type_enemy, 2, 4));
    spawn_entity(new_entity(entity_type_zombie, 3, 4));

    for (int i = 0; i < 10; i++) {
        spawn_entity(new_entity(entity_type_stone, 2 + i, 6));
        spawn_entity(new_entity(entity_type_stone, 3 + i, 7));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 8));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 9));
        spawn_entity(new_entity(entity_type_stone, 4 + i, 10));
    }


    //
    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }
    else if (!main_player) {
        printf("The main player is NULL!\n");
        return 1;
    } 
    else if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    } 
    else if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }
    else {
        window = SDL_CreateWindow("CaveKingdom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);
        
        font = TTF_OpenFont("./assets/fonts/roboto.ttf", 36);

        if (font == NULL)
        {
            printf("Failed to load roboto font! SDL_ttf Error: %s\n", TTF_GetError());
            return 1;
        }
        else if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }
        else {
            init_rendering();
            load_textures();

            int quit = 0;

            int player_movement_x, player_movement_y;
            int w_key_pressed = 0;
            int s_key_pressed = 0;
            int a_key_pressed = 0;
            int d_key_pressed = 0;

            SDL_Event event;
            while (quit == 0) { 
                player_movement_x = 0;
                player_movement_y = 0;

                while (SDL_PollEvent(&event)) { 
                    if (event.type == SDL_QUIT) quit = 1; 
                    else if (event.type == SDL_KEYDOWN) {
                        if (SDLK_w == event.key.keysym.sym) w_key_pressed = 1;
                        else if (SDLK_s == event.key.keysym.sym) s_key_pressed = 1;
                        else if (SDLK_a == event.key.keysym.sym) a_key_pressed = 1;
                        else if (SDLK_d == event.key.keysym.sym) d_key_pressed = 1;
                        else if (SDLK_e == event.key.keysym.sym) main_camera.zoom = SDL_clamp(main_camera.zoom + .1, main_camera.min_zoom, main_camera.max_zoom);
                        else if (SDLK_q == event.key.keysym.sym) main_camera.zoom = SDL_clamp(main_camera.zoom - .1, main_camera.min_zoom, main_camera.max_zoom);
                        else if (SDLK_LEFT == event.key.keysym.sym) main_camera.x -= 10;
                        else if (SDLK_RIGHT == event.key.keysym.sym) main_camera.x += 10;
                        else if (SDLK_UP == event.key.keysym.sym) main_camera.y -= 10;
                        else if (SDLK_DOWN == event.key.keysym.sym) main_camera.y += 10;

                        //else if (SDLK_r == event.key.keysym.sym) print_inventory(&main_player->inventory);

                        
                    }
                    else if (event.type == SDL_KEYUP) {
                        if (SDLK_w == event.key.keysym.sym) w_key_pressed = 0;
                        else if (SDLK_s == event.key.keysym.sym) s_key_pressed = 0;
                        else if (SDLK_a == event.key.keysym.sym) a_key_pressed = 0;
                        else if (SDLK_d == event.key.keysym.sym) d_key_pressed = 0;
                    }
                } 
                if (w_key_pressed) player_movement_y--;
                if (s_key_pressed) player_movement_y++;
                if (a_key_pressed) player_movement_x--;
                if (d_key_pressed) player_movement_x++;

                update_entities(player_movement_x, player_movement_y);
                update_camera();
                draw_world();

                SDL_Delay(100);
            }
        }
    }


    unload_textures();

    TTF_CloseFont(font);

    SDL_DestroyWindow(window);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}