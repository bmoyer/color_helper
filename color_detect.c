#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "color_detect.h"

color nearest_color(int r, int g, int b, color colors[]) {
    color c;
    
    c.r = 1;
    c.g = 2;
    c.b = 3;
    return c;
}

color* read_colors() {
    color* ret = malloc(sizeof(color) * NUM_COLORS);
    int i = 0;
    color c;

    FILE* file = fopen("map.txt", "r");
    while (fgets(c.name, 30, file)) {
        ret[i] = c;
        i++;
    }
    fclose(file);
/*
    for(int i = 0; i < NUM_COLORS; i++) {
        color c;
        c.r = 1;
        c.g = 2;
        c.b = 3;
        strcpy(c.name, "Banana Yellow");
        ret[i] = c;
    }
    */
    return ret;
}

