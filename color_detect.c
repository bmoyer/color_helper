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
