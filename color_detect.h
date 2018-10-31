#ifndef COLOR_DETECT_H_INCLUDED
#define COLOR_DETECT_H_INCLUDED

#define NUM_COLORS 255

typedef struct {
    int r;
    int g;
    int b;
    char name[30];
} color;

color nearest_color(int r, int g, int b, color* colors, int ncolors);
color* read_colors();

#endif /*COLOR_DETECT_H_INCLUDED*/

