#pragma once

typedef struct {
    int x;
    int y;
    float zoom;
    float min_zoom;
    float max_zoom;
} Camera;

extern Camera main_camera;