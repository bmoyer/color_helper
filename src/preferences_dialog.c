#include <gtk/gtk.h>

#include "preferences_dialog.h"

//static void on_preferences(GtkWidget* menu_item, gpointer userdata) {
void on_rgb_toggled(gpointer userdata) {
    /*
    preferences* prefs = (preferences*) userdata;
    prefs->rgb_display = 1 - prefs->rgb_display;
    printf("prefs.rgb_display=%d", prefs->rgb_display);
    */
}

void show_preferences_dialog(preferences* prefs) {
    GtkWidget* dialog = gtk_dialog_new();
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    GtkWidget* rgb_check = gtk_check_button_new_with_label("RGB display");
    gtk_box_pack_start(GTK_BOX(vbox), rgb_check, 0, 0, 0);

    GtkWidget* hex_check = gtk_check_button_new_with_label("Hex display");
    gtk_box_pack_start(GTK_BOX(vbox), hex_check, 0, 0, 0);

    g_signal_connect(rgb_check, "toggled", (GCallback)on_rgb_toggled, prefs);

    display_preferences(prefs);

    gtk_widget_show_all(dialog);
}

void display_preferences(preferences* prefs) {
}
