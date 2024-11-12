#include <stdio.h>

#include "world/chunk.h"
#include "graphics/vector.h"
#include "graphics/camera.h"

Vector2 from_screen_to_tile_coords(Vector2 screen_coords) {
    Vector2f screen_coords_f = vector2_to_f(screen_coords);

    Vector2f camera_f = vector2_to_f((Vector2) { camera.x, camera.y });

    Vector2f adjusted_coords = vector2f_sum(screen_coords_f, camera_f);

    Vector2f tile_size_f = { TILE_SIZE * camera.zoom, TILE_SIZE * camera.zoom };

    Vector2f tile_coords_f = vector2f_divide(adjusted_coords, tile_size_f);

    return vector2f_to_i(tile_coords_f);
}

