#ifndef PREFERENCES_H_INCLUDED
#define PREFERENCES_H_INCLUDED

#include <glib.h>

typedef struct {
    int rgb_display;
    int hex_display;
    int hsv_display;
    int name_display;
    int title_bar;

} preferences;

preferences* preferences_read();
void preferences_write(preferences* preferences);
void print(preferences* preferences);
gchar* get_preferences_file_path();

#endif
