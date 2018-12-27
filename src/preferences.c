#include "preferences.h"
#include <stdlib.h>
#include <stdio.h>

preferences* read_preferences() {
    preferences* prefs = malloc(sizeof(preferences));

    prefs->rgb_display = 1;
    prefs->hex_display = 0;
    prefs->hsv_display = 0;
    prefs->name_display = 1;
    prefs->title_bar= 1;

    return prefs;
}

void print(preferences* prefs) {
    printf("preferences: %d %d %d %d\n", prefs->rgb_display, prefs->hex_display, prefs->hsv_display, prefs->title_bar);
}
