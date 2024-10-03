#include "vector.h"

Vector2 vector2_sum(Vector2 vector_a, Vector2 vector_b) {
	return (Vector2) { vector_a.x + vector_b.x, vector_a.y + vector_b.y };
}

Vector2f vector2f_sum(Vector2f vector_a, Vector2f vector_b) {
	return (Vector2f) { vector_a.x + vector_b.x, vector_a.y + vector_b.y };
}