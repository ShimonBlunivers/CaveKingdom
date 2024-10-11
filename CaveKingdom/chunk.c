#include <stdio.h>

#include "chunk.h"
#include "vector.h"
#include "camera.h"

Vector2 from_screen_to_tile_coords(Vector2 screen_coords) {
	return vector2_divide(vector2_sum(screen_coords, (Vector2) { camera.x, camera.y }), (Vector2) { TILE_SIZE* camera.zoom, TILE_SIZE* camera.zoom });
}
