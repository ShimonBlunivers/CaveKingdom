#pragma once

#include <stdbool.h>

typedef struct Vector2 {
    int x;
    int y;
} Vector2;

typedef struct Vector2f {
    float x;
    float y;
} Vector2f;


Vector2 vector2_sum(Vector2 vector_a, Vector2 vector_b);
Vector2 vector2_subtract(Vector2 vector_a, Vector2 vector_b);
Vector2 vector2_multiplicate(Vector2 vector_a, Vector2 vector_b);
Vector2 vector2_divide(Vector2 vector_a, Vector2 vector_b);
bool vector2_equals(Vector2 vector_a, Vector2 vector_b);
Vector2 vector2f_to_i(Vector2f vector_f);

//Vector2 vector2_normalize(Vector2 vector);

Vector2f vector2f_sum(Vector2f vector_a, Vector2f vector_b);
Vector2f vector2f_subtract(Vector2f vector_a, Vector2f vector_b);
Vector2f vector2f_multiplicate(Vector2f vector_a, Vector2f vector_b);
Vector2f vector2f_divide(Vector2f vector_a, Vector2f vector_b);
bool vector2f_equals(Vector2f vector_a, Vector2f vector_b);
Vector2f vector2f_normalize(Vector2f vector);
Vector2f vector2_to_f(Vector2 vector_i);
