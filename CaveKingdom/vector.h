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
Vector2f vector2f_sum(Vector2f vector_a, Vector2f vector_b);