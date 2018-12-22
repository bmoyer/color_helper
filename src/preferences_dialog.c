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

void show_preferences_dialog(GtkWindow* parent, preferences* prefs) {
    GtkWidget* dialog = gtk_dialog_new();
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget* notebook = gtk_notebook_new();
    add_view_tab(notebook, prefs);
    gtk_container_add(GTK_CONTAINER(content_area), notebook);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    display_preferences(prefs);

    gtk_widget_show_all(dialog);
}

void add_view_tab(GtkWidget* notebook, preferences* prefs) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget* viewTabLabel = gtk_label_new("View");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, viewTabLabel);

    GtkWidget* rgb_check = gtk_check_button_new_with_label("RGB display");
    gtk_box_pack_start(GTK_BOX(vbox), rgb_check, 0, 0, 0);

    GtkWidget* hex_check = gtk_check_button_new_with_label("Hex display");
    gtk_box_pack_start(GTK_BOX(vbox), hex_check, 0, 0, 0);

    GtkWidget* hsv_check = gtk_check_button_new_with_label("HSV display");
    gtk_box_pack_start(GTK_BOX(vbox), hsv_check, 0, 0, 0);

    GtkWidget* title_bar_check = gtk_check_button_new_with_label("Title bar");
    gtk_box_pack_start(GTK_BOX(vbox), title_bar_check, 0, 0, 0);

    g_signal_connect(rgb_check, "toggled", (GCallback)on_rgb_toggled, prefs);
}

void display_preferences(preferences* prefs) {
}
