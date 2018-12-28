#include "preferences.h"
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

preferences* preferences_read() {
    GKeyFile* file = g_key_file_new();
    preferences* prefs = malloc(sizeof(preferences));
    g_key_file_load_from_file(file, get_preferences_file_path(), G_KEY_FILE_NONE, NULL);

    prefs->rgb_display = g_key_file_get_integer(file, "View", "rgb_display", NULL);
    prefs->hex_display = g_key_file_get_integer(file, "View", "hex_display", NULL);
    prefs->hsv_display = g_key_file_get_integer(file, "View", "hsv_display", NULL);
    prefs->name_display = g_key_file_get_integer(file, "View", "name_display", NULL);
    prefs->title_bar = g_key_file_get_integer(file, "View", "title_bar", NULL);

    g_key_file_free(file);

    return prefs;
}

void print(preferences* prefs) {
    printf("preferences: %d %d %d %d\n", prefs->rgb_display, prefs->hex_display, prefs->hsv_display, prefs->title_bar);
}

gchar* get_preferences_file_path() {
    const gchar* dir = g_get_user_config_dir();
    gchar* config_file = g_build_path("/", dir, "colorhelperrc", NULL);

    return config_file;
}

void preferences_write(preferences* prefs) {
    GKeyFile* file = g_key_file_new();

    g_key_file_set_integer (file,
                        "View",
                        "rgb_display",
                        prefs->rgb_display);

    g_key_file_set_integer (file,
                        "View",
                        "hex_display",
                        prefs->hex_display);

    g_key_file_set_integer (file,
                        "View",
                        "hsv_display",
                        prefs->hex_display);

    g_key_file_set_integer (file,
                        "View",
                        "name_display",
                        prefs->name_display);

    g_key_file_set_integer (file,
                        "View",
                        "title_bar",
                        prefs->title_bar);

    g_key_file_save_to_file(file, get_preferences_file_path(), NULL);
    g_key_file_free(file);
}

