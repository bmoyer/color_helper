#ifndef PREFERENCES_H_INCLUDED
#define PREFERENCES_H_INCLUDED

#include <glib.h>

typedef struct {
    int rgb_display;
    int hex_display;
    int hsv_display;
    int name_display;
    int title_bar;
    int draw_crosshair;

} preferences;

preferences* preferences_read();
void preferences_write(preferences* preferences);
void preferences_print(preferences* preferences);

#endif
