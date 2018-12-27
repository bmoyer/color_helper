#ifndef PREFERENCES_H_INCLUDED
#define PREFERENCES_H_INCLUDED

typedef struct {
    int rgb_display;
    int hex_display;
    int hsv_display;
    int name_display;
    int title_bar;

} preferences;

preferences* read_preferences();
void print(preferences* preferences);

#endif
