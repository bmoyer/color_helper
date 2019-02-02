#ifndef COLOR_DETECT_H_INCLUDED
#define COLOR_DETECT_H_INCLUDED

typedef struct {
    // RGB color space
    int r;
    int g;
    int b;

    // YUV color space
    int y;
    int u;
    int v;

    char name[60];
} color;

void yuv_from_rgb(int* y, int* u, int* v, int r, int g, int b);
color nearest_color(int r, int g, int b, color* colors, int ncolors);
color* read_colors();

#endif /*COLOR_DETECT_H_INCLUDED*/

