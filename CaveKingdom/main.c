#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#define MAP_WIDTH 32
#define MAP_HEIGHT 16

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int game_status = 1;

typedef enum {
    entity_type_empty = 0,
    entity_type_player = 1,
    entity_type_wall = 2,
    entity_type_enemy = 4,
} EntityType;

typedef struct {
    EntityType entity_type;
    int x;
    int y;
    int max_health;
    int health;
} Entity;

static Entity entities[MAP_WIDTH * MAP_HEIGHT];
static Entity* grid[MAP_WIDTH * MAP_HEIGHT];

Entity new_empty(int x, int y) {
    return (Entity) { entity_type_empty, x, y, -1, -1 };
}

void reset_grids() {
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
        entities[i] = new_empty(i % MAP_WIDTH, i / MAP_WIDTH);
        grid[i] = &entities[i];
    }
}

Entity new_player(int x, int y) {
    return (Entity) { entity_type_player, x, y, 100, 100 };
}

Entity new_wall(int x, int y) {
    return (Entity) { entity_type_wall, x, y, -1, -1 };
}

Entity new_enemy(int x, int y) {
    return (Entity) { entity_type_enemy, x, y, 100, 100 };
}

Entity* get_entity(int x, int y) {
    // printf("X: %d\n", x);
    // printf("Y: %d\n", y);
    return grid[y * MAP_WIDTH + x];
}

void set_entity(int x, int y, Entity* enity) {
    grid[y * MAP_WIDTH + x] = enity;
}

int spawn_entity(Entity entity) {
    Entity* old_entity = get_entity(entity.x, entity.y);
    if (old_entity->entity_type != entity_type_empty) return 0;
    *old_entity = entity;
    return 1;
}

int switch_entities(int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 >= MAP_WIDTH || x2 >= MAP_WIDTH || y1 >= MAP_HEIGHT || y2 >= MAP_HEIGHT) return 0;

    Entity* entity1 = get_entity(x1, y1);
    Entity* entity2 = get_entity(x2, y2);

    if (entity1->entity_type == entity_type_wall || entity2->entity_type == entity_type_wall) return 0;

    entity2->x = x1;
    entity2->y = y1;

    entity1->x = x2;
    entity1->y = y2;

    Entity* temp = entity1;
    set_entity(x1, y1, entity2);
    set_entity(x2, y2, temp);

    return 1;
}


void update_world(char input) {
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++)
    {
        Entity* entity = &entities[i];
        if (entity->entity_type == entity_type_empty) {}
        else if (entity->entity_type == entity_type_wall) {}
        else if (entity->entity_type == entity_type_player) {
            if (input == 'w') {
                switch_entities(entity->x, entity->y, entity->x, entity->y - 1);
            }
            else if (input == 's') {
                switch_entities(entity->x, entity->y, entity->x, entity->y + 1);
            }
            else if (input == 'a') {
                switch_entities(entity->x, entity->y, entity->x - 1, entity->y);
            }
            else if (input == 'd') {
                switch_entities(entity->x, entity->y, entity->x + 1, entity->y);
            }
        }
        else if (entity->entity_type == entity_type_enemy) {

        }
    }
}

void render_world() {
    char border = 'X';

    char frame[(MAP_WIDTH + 2 + 1) * (MAP_HEIGHT + 2)];
    for (int i = 0; i < (MAP_WIDTH + 2 + 1) * (MAP_HEIGHT + 2); i++) frame[i] = ' ';
    frame[(MAP_WIDTH + 2 + 1) * (MAP_HEIGHT + 2) - 1] = '\0';

    for (int i = 0; i < MAP_WIDTH + 2; i++) frame[i] = border;
    frame[MAP_WIDTH + 2] = '\n';

    for (int y = 0; y < MAP_HEIGHT; y++) {
        frame[(MAP_WIDTH + 3) * (y + 1)] = border;
        for (int x = 0; x < MAP_WIDTH; x++) {
            Entity entity = *get_entity(x, y);
            if (entity.entity_type == entity_type_wall) {
                frame[(MAP_WIDTH + 3) * (y + 1) + 1 + x] = 'O';
            }
            else if (entity.entity_type == entity_type_player) {
                frame[(MAP_WIDTH + 3) * (y + 1) + 1 + x] = 'P';
            }
            else if (entity.entity_type == entity_type_enemy) {
                frame[(MAP_WIDTH + 3) * (y + 1) + 1 + x] = 'E';
            }
        }
        frame[(MAP_WIDTH + 3) * (y + 1) + MAP_WIDTH + 1] = border;
        frame[(MAP_WIDTH + 3) * (y + 1) + MAP_WIDTH + 2] = '\n';
    }
    for (int i = 2; i < MAP_WIDTH + 4; i++) {
        frame[(MAP_WIDTH + 3) * (MAP_HEIGHT + 2) - i] = border;
    }

    system("cls");
    printf("\n%s", frame);
}



int main(int argc, char* argv[]) {

    // reset_grids();

    // spawn_entity(new_player(0, 1));
    // spawn_entity(new_enemy(0, 4));
    // spawn_entity(new_wall(2, 4));

    // while (game_status) {
    //     render_world();

    //     char input = ' ';

    //     scanf("%c", &input);

    //     update_world(input);

    // }

    printf("STARTED!");

    //The window we'll be rendering to
    SDL_Window* window = NULL;

    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        //Create window
        window = SDL_CreateWindow("CaveKingdom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            //Get window surface
            screenSurface = SDL_GetWindowSurface(window);

            //Fill the surface white
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x25, 0xFF, 0xFF));

            //Update the surface
            SDL_UpdateWindowSurface(window);

            //Hack to get window to stay up

            SDL_Event e;
            
            int quit = 0;
            
            while (quit == 0) { 
                

                while (SDL_PollEvent(&e)) { 
                    if (e.type == SDL_QUIT) quit = 1; 
                } 

                //SDL_RenderClear(screenSurface); // Asi? Psal jsem to já, takže nevím Xdd

                SDL_Rect rect;
                rect.x = 250;
                rect.y = 150;
                rect.w = 200;
                rect.h = 200;

                SDL_SetRenderDrawColor(screenSurface, 255, 255, 255, 255);
                SDL_RenderDrawRect(screenSurface, &rect);

                SDL_SetRenderDrawColor(screenSurface, 0, 0, 0, 255);

                SDL_RenderPresent(screenSurface);
            }
        }
    }
    //Destroy window
    SDL_DestroyWindow(window);

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
}