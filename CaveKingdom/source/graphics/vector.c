#include <stdio.h>
#include <math.h>

#include "vector.h"

Vector2 vector2_sum(Vector2 vector_a, Vector2 vector_b) {
	return (Vector2) { vector_a.x + vector_b.x, vector_a.y + vector_b.y };
}

Vector2 vector2_subtract(Vector2 vector_a, Vector2 vector_b) {
	return (Vector2) { vector_a.x - vector_b.x, vector_a.y - vector_b.y };
}

Vector2 vector2_multiplicate(Vector2 vector_a, Vector2 vector_b) {
	return (Vector2) { vector_a.x * vector_b.x, vector_a.y * vector_b.y };
}
Vector2 vector2f_to_i(Vector2f vector_f) {
	return (Vector2) { (int)(vector_f.x), (int)(vector_f.y) };
}



Vector2 vector2_divide(Vector2 vector_a, Vector2 vector_b) {
	if (vector_b.x == 0 || vector_b.y == 0) {
		printf("vector2_divide exception: Attempted dividing by zero!");
		return (Vector2) { 0, 0 };
	}
	return (Vector2) { round(vector_a.x / vector_b.x), round(vector_a.y / vector_b.y) };
}

Vector2f vector2_to_f(Vector2 vector_i) {
	return (Vector2f) { (float)vector_i.x, (float)vector_i.y };
}
Vector2f vector2f_sum(Vector2f vector_a, Vector2f vector_b) {
	return (Vector2f) { vector_a.x + vector_b.x, vector_a.y + vector_b.y };
}

Vector2f vector2f_subtract(Vector2f vector_a, Vector2f vector_b) {
	return (Vector2f) { vector_a.x - vector_b.x, vector_a.y - vector_b.y };
}

Vector2f vector2f_multiplicate(Vector2f vector_a, Vector2f vector_b) {
	return (Vector2f) { vector_a.x * vector_b.x, vector_a.y * vector_b.y };
}

Vector2f vector2f_divide(Vector2f vector_a, Vector2f vector_b) {
	if (vector_b.x == 0 || vector_b.y == 0) {
		printf("vector2f_divide exception: Attempted dividing by zero!");
		return (Vector2f) { 0., 0. };
	}
	return (Vector2f) { vector_a.x / vector_b.x, vector_a.y / vector_b.y };
}

bool vector2_equals(Vector2 vector_a, Vector2 vector_b) {
	return vector_a.x == vector_b.x && vector_a.y == vector_b.y ;
}
bool vector2f_equals(Vector2f vector_a, Vector2f vector_b) {
	return vector_a.x == vector_b.x && vector_a.y == vector_b.y;
}

//Vector2 vector2_normalize(Vector2 vector) {
//    float magnitude = sqrt(vector.x * vector.x + vector.y * vector.y);
//    if (magnitude == 0) return (Vector2) { 0, 0 };
//    return (Vector2) {(int)(vector.x / magnitude), (int)(vector.y / magnitude)};
//}

Vector2f vector2f_normalize(Vector2f vector) {
    float magnitude = sqrt(vector.x * vector.x + vector.y * vector.y);
    if (magnitude == 0.0f) return (Vector2f) { 0.0f, 0.0f };
    return (Vector2f) { vector.x / magnitude, vector.y / magnitude};
}