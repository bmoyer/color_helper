#ifndef PREFERENCES_DIALOG_H_INCLUDED
#define PREFERENCES_DIALOG_H_INCLUDED

#include "preferences.h"

void show_preferences_dialog(GtkWindow* parent, preferences* prefs, gboolean(* on_preferences_closed_func)(GtkWidget*, GdkEvent*, gpointer));
void display_preferences(preferences* pref);
void add_view_tab(GtkWidget* notebook, preferences* prefs);
void add_color_tab(GtkWidget* notebook, preferences* prefs);
void on_option_toggled(GtkToggleButton* widget, gpointer userdata);

#endif 
