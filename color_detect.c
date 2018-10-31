#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "color_detect.h"

int get_color_distance(int r, int g, int b, color* c) {
    return abs(r-c->r) + abs(g-c->g) + abs(b-c->b);
}

color nearest_color(int r, int g, int b, color* colors, int ncolors) {
    color closest = colors[0];
    int smallest_dist = get_color_distance(r, g, b, &closest);

    for(int i = 1; i < ncolors; i++) {
        int dist = get_color_distance(r, g, b, &colors[i]);
        if(dist < smallest_dist) {
            smallest_dist = dist;
            closest = colors[i];
        }
    }

    return closest;
}

color* read_colors() {
    color* ret = malloc(sizeof(color) * NUM_COLORS);
    int i = 0;
    color c;

    FILE* file = fopen("map.txt", "r");
    char line[50];
    while (fgets(line, 50, file)) {
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

        ret[i] = c;
        i++;
    }
    fclose(file);

    return ret;
}

