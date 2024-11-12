#pragma once

typedef struct {
    int x;
    int y;
    double zoom;
    double min_zoom;
    double max_zoom;
} Camera;

extern Camera camera;

void update_camera();