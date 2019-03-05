#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "color_detect.h"

#ifndef MAX_COLORS
#define MAX_COLORS 5000
#endif

int get_color_distance(int r, int g, int b, color* c) {
    long rmean = ((long)r + (long)c->r)/2;
    long r2 = (long)r - (long)c->r;
    long g2 = (long)g - (long)c->g;
    long b2 = (long)b - (long)c->b;

    return sqrt((((512+rmean)*r2*r2)>>8) + 4*g2*g2 + (((767-rmean)*b2*b2)>>8));
}

// from https://www.compuphase.com/cmetric.htm
color nearest_color(int y, int u, int v, color* colors, int ncolors) {
    color closest = colors[0];
    int smallest_dist = get_color_distance(y, u, v, &closest);

    for(int i = 1; i < ncolors; i++) {
        int dist = get_color_distance(y, u, v, &colors[i]);
        if(dist < smallest_dist) {
            smallest_dist = dist;
            closest = colors[i];
        }
    }

    //printf("closest (rgb): (%d,%d,%d)\n", closest.r,closest.g,closest.b);
    //printf("yuvs: (%d,%d,%d) --- (%d,%d,%d)\n",
        //y,u,v,closest.r,closest.g,closest.b);
    return closest;
}

void yuv_from_rgb(int* y, int* u, int* v, int r, int g, int b) {
    *y =  0.257 * r + 0.504 * g + 0.098 * b +  16;
    *u = -0.148 * r - 0.291 * g + 0.439 * b + 128;
    *v =  0.439 * r - 0.368 * g - 0.071 * b + 128;
}

int read_colors(color** color_list, char* filepath, int* num_colors) {
    int i = 0;
    color c;

    FILE* file;
    if((file = fopen(filepath, "r")) == NULL) {
        sprintf(c.name, "Unknown");
        c.r = c.g = c.b = 0;
        *color_list[0] = c;
        return 0;
    }

    *color_list = calloc(MAX_COLORS, sizeof(color));

    char line[60];
    while (fgets(line, 60, file) && i < MAX_COLORS) {
        // read color name
        char* pt = strtok(line, ",");
        sprintf(c.name, "%s", pt);
        
        // read red value
        pt = strtok(NULL, ",");
        c.r = atoi(pt);

        // read green value
        pt = strtok(NULL, ",");
        c.g = atoi(pt);

        // read blue value
        pt = strtok(NULL, ",");
        c.b = atoi(pt);

        yuv_from_rgb(&c.y, &c.u, &c.v, c.r, c.g, c.b);

        (*color_list)[i] = c;
        i++;
    }
    *num_colors = i;
    fclose(file);

    return 1;
}

