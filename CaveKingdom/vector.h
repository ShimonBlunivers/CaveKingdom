#pragma once


typedef struct {
    int x;
    int y;
} Vector2;

typedef struct {
    float x;
    float y;
} Vector2f;


Vector2 vector2_sum(Vector2 vector_a, Vector2 vector_b);
bool vector2_equals(Vector2 vector_a, Vector2 vector_b);
//Vector2 vector2_normalize(Vector2 vector);

Vector2f vector2f_sum(Vector2f vector_a, Vector2f vector_b);
bool vector2f_equals(Vector2f vector_a, Vector2f vector_b);
Vector2f vector2f_normalize(Vector2f vector);
