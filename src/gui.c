#include <gtk/gtk.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include "preferences_dialog.h"
#include "preferences.h"
#include "color_detect.h"

#define MAX_COLORS   1000
#define CONTEXT_SIZE 20
#define CONTEXT_DISPLAY_SIZE 100

static Window root;
static cairo_surface_t *rgb_surface = NULL;
static cairo_surface_t *context_surface = NULL;

Display* d;

GtkWidget *main_window;
GtkWidget* color_name_label;
GtkWidget* rgb_label;
GtkWidget* hex_label;
GtkWidget *color_drawing_area;
GtkWidget *context_drawing_area;
GMainContext *context;
GThread* update_thread;
preferences app_preferences;

int running = 1;
GMutex running_mutex;

color CONTEXT_BUFFER[CONTEXT_SIZE][CONTEXT_SIZE];
color* colors;

void refresh_preferences() {
    app_preferences.rgb_display ? gtk_widget_show(rgb_label) :
                                  gtk_widget_hide(rgb_label);

    app_preferences.hex_display ? gtk_widget_show(hex_label) :
                                  gtk_widget_hide(hex_label);

    app_preferences.name_display ? gtk_widget_show(color_name_label) :
                                  gtk_widget_hide(color_name_label);

    gtk_window_set_decorated((GtkWindow*)main_window, app_preferences.title_bar);
}

static gboolean on_preferences_closed(GtkWidget* widget, GdkEvent* event, gpointer user_data) {
    preferences_write(&app_preferences);
    refresh_preferences();

    return FALSE;
}

static void on_preferences(GtkWidget* menu_item, gpointer userdata) {
    // create prefs object from on-disk prefs
    // object comes back to on_preferences_closed, and is then saved/swapped in
    show_preferences_dialog((GtkWindow*)main_window, &app_preferences, on_preferences_closed);
}

static void on_quit(GtkWidget* menu_item, gpointer userdata) {
    //gtk_window_close((GtkWindow*)&root);
    exit(0);
}

static void clear_surface (cairo_surface_t* surface) {
    cairo_t *cr;

    cr = cairo_create (surface);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_destroy (cr);
}

static void view_popup_menu(GtkWidget* widget, GdkEventButton* event, gpointer userdata) {
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *menuitem = gtk_menu_item_new_with_label("Preferences...");
    GtkWidget *close_item = gtk_menu_item_new_with_label("Close");

    g_signal_connect(menuitem, "activate", (GCallback)on_preferences, NULL);
    g_signal_connect(close_item, "activate", (GCallback)on_quit, NULL);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), close_item);
    gtk_widget_show_all(menu);
    gtk_menu_popup_at_pointer(GTK_MENU(menu), (const GdkEvent*) event);
}

static gboolean on_window_clicked(GtkWidget* widget, GdkEventButton* event, gpointer userdata)
{
    if (event->type == GDK_BUTTON_PRESS && event->button == 3){
        view_popup_menu(widget, NULL, userdata);
    }

    if (event->type == GDK_BUTTON_PRESS && event->button == 1){
        gtk_window_begin_move_drag(GTK_WINDOW(widget), event->button, event->x_root, event->y_root, event->time);
    }

    return TRUE;
}

static gboolean configure_event_cb_rgb (GtkWidget *widget,
                                        GdkEventConfigure *event,
                                        gpointer           data) {
    if (rgb_surface)
        cairo_surface_destroy (rgb_surface);

    rgb_surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
            CAIRO_CONTENT_COLOR,
            gtk_widget_get_allocated_width 
            (widget),
            gtk_widget_get_allocated_height 
            (widget));

    clear_surface (rgb_surface);
    return TRUE;
}

static gboolean configure_event_cb_context (GtkWidget *widget,
                                            GdkEventConfigure *event,
                                            gpointer data) {
    if (context_surface)
        cairo_surface_destroy (context_surface);

    context_surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
            CAIRO_CONTENT_COLOR,
            gtk_widget_get_allocated_width 
            (widget),
            gtk_widget_get_allocated_height 
            (widget));

    clear_surface (context_surface);
    return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean draw_cb_rgb (GtkWidget *widget,
                             cairo_t   *cr,
                            gpointer   data) {
    cairo_set_source_surface (cr, rgb_surface, 0, 0);
    cairo_paint (cr);

    return FALSE;
}

static gboolean draw_cb_context (GtkWidget *widget, 
                                 cairo_t *cr, 
                                 gpointer data) {
    cairo_set_source_surface (cr, context_surface, 0, 0);
    cairo_paint (cr);

    return FALSE;
}

static void draw_rect(int r, int g, int b) {
    cairo_t *cr;
    /* Paint to the surface, where we store our state */
    cr = cairo_create (rgb_surface);
    cairo_set_source_rgb(cr, r/255.0, g/255.0, b/255.0);

    cairo_rectangle (cr, 0, 0, CONTEXT_DISPLAY_SIZE, CONTEXT_DISPLAY_SIZE);
    cairo_fill (cr);

    cairo_destroy (cr);

    /* Now invalidate the affected region of the drawing area. */
    gtk_widget_queue_draw_area (color_drawing_area, 0, 0, CONTEXT_DISPLAY_SIZE, CONTEXT_DISPLAY_SIZE);
}

static void draw_crosshair(cairo_t* rect) {
            cairo_set_source_rgb(rect, 1.0,1.0,1.0);
            int SPACING = 10;

            // draw vertical dotted line
            for(int i = 0; i < CONTEXT_DISPLAY_SIZE/SPACING; i++) {
                int half = CONTEXT_DISPLAY_SIZE/2;
                cairo_rectangle(rect, half, i*SPACING, 1, 5);
            }

            // draw horizontal dotted line
            for(int i = 0; i < CONTEXT_DISPLAY_SIZE/SPACING; i++) {
                int half = CONTEXT_DISPLAY_SIZE/2;
                cairo_rectangle(rect, i*SPACING, half, 5, 1);
            }

            cairo_fill(rect);
}

static void draw_context_pixels() {
    cairo_t *cr;
    cr = cairo_create (context_surface);

    int PIXEL_SCALE = CONTEXT_DISPLAY_SIZE/CONTEXT_SIZE;

    for(int x = 0; x < CONTEXT_SIZE; x++) {
        for(int y = 0; y < CONTEXT_SIZE; y++) {
            if(app_preferences.draw_crosshair) {
                draw_crosshair(cr);
            }
            cairo_set_source_rgb(cr,
                    CONTEXT_BUFFER[x][y].r/255.0, 
                    CONTEXT_BUFFER[x][y].g/255.0, 
                    CONTEXT_BUFFER[x][y].b/255.0);
            cairo_rectangle(cr, x*PIXEL_SCALE, y*PIXEL_SCALE, PIXEL_SCALE, PIXEL_SCALE);
            cairo_fill(cr);
        }
    }

    cairo_destroy (cr);
    gtk_widget_queue_draw_area (context_drawing_area, 0, 0, CONTEXT_DISPLAY_SIZE, CONTEXT_DISPLAY_SIZE);
}

static gboolean delete_event(GtkWidget* w, GdkEvent* e, gpointer d) {
    g_mutex_lock(&running_mutex);
    running = 0;
    g_mutex_unlock(&running_mutex);
    return FALSE;
}

static void close_window (void) {
    if (rgb_surface)
        cairo_surface_destroy (rgb_surface);

    if (context_surface)
        cairo_surface_destroy (context_surface);
}

static void query_pointer(int* x, int* y) {
    static int once;
    int i;
    unsigned m;
    Window w;

    if (once == 0) {
        once = 1;
        root = DefaultRootWindow(d);
    }

    if (!XQueryPointer(d, root, &root, &w, x, y, &i, &i, &m)) {
        for (i = -1; ++i < ScreenCount(d); ) {
            if (root == RootWindow(d, i)) {
                break;
            } 
        }
    }
}

void get_color(Display* d, int x, int y, int* r, int* g, int* b) {
    XColor c;
    XImage *image = XGetImage (d, XRootWindow (d, XDefaultScreen (d)), x, y, 1, 1, AllPlanes, XYPixmap);
    c.pixel = XGetPixel (image, 0, 0);
    XFree (image);
    XQueryColor (d, XDefaultColormap(d, XDefaultScreen (d)), &c);

    *r = (c.red/256);
    *b = (c.blue/256);
    *g = (c.green/256);
}

void clip_coords_to_display_size(int* x, int* y) {
    Screen* screen = ScreenOfDisplay(d, 0);
    if (*x < 0) {
        *x = 0;
    }
    if (*x >= screen->width) {
        *x = screen->width-1;
    }

    if (*y < 0) {
        *y = 0;
    }
    if (*y >= screen->height) {
        *y = screen->height-1;
    }
}

void get_context_pixels(Display* d, int x, int y) {
    for (int i = 0; i < CONTEXT_SIZE; i++) {
        for(int j = 0; j < CONTEXT_SIZE; j++) {
            int mouse_x = x - i + CONTEXT_SIZE/2;
            int mouse_y = y - j + CONTEXT_SIZE/2;
            clip_coords_to_display_size(&mouse_x, &mouse_y);
            get_color(d, mouse_x, mouse_y, &CONTEXT_BUFFER[CONTEXT_SIZE-1-i][CONTEXT_SIZE-1-j].r, &CONTEXT_BUFFER[CONTEXT_SIZE-1-i][CONTEXT_SIZE-1-j].g, &CONTEXT_BUFFER[CONTEXT_SIZE-1-i][CONTEXT_SIZE-1-j].b);
        }
    }
}

static gboolean update_color(gpointer user_data) {
    if(!running)
        return G_SOURCE_REMOVE;

    int r, g, b, x, y;
    query_pointer(&x, &y);
    char s2[50] = "";

    // get color of pixel under cursor
    get_color(d, x, y, &r, &g, &b);

    // get pixels around cursor
    get_context_pixels(d, x, y);
    draw_context_pixels();

    // set RGB readout
    sprintf(s2, "RGB: (%03d, %03d, %03d)", r, g, b);

    char *rgb_str = g_strdup_printf ("<span font=\"12\" color=\"black\">"
                               "%s"
                             "</span>",
                             s2);

    gtk_label_set_markup(GTK_LABEL(rgb_label), rgb_str);
    // set name readout
    color c = nearest_color(r, g, b, colors, MAX_COLORS);

    char nameLbl[60];
    sprintf(nameLbl, c.name);
    char *name_str = g_strdup_printf ("<span font=\"12\" color=\"black\">"
                               "%s"
                             "</span>",
                             nameLbl);
    gtk_label_set_markup(GTK_LABEL(color_name_label), name_str);

    char *hex_str = g_strdup_printf ("<span font=\"12\" color=\"black\">"
                               "Hex: %03X %03X %03X"
                             "</span>",
                             r, g, b);
    gtk_label_set_markup(GTK_LABEL(hex_label), hex_str);
    draw_rect(r, g, b);

    g_free(rgb_str);
    g_free(name_str);
    g_free(hex_str);

    return G_SOURCE_REMOVE;
}

static gpointer update_thread_func (gpointer user_data) {
    GSource *source;

    for (;;) {
        g_mutex_lock(&running_mutex);
        if(!running) {
            g_mutex_unlock(&running_mutex);
            g_thread_exit(NULL);
        }
        g_mutex_unlock(&running_mutex);
        source = g_idle_source_new();
        g_source_set_callback(source, update_color, NULL, NULL);
        g_source_attach(source, context);
        g_source_unref(source);
        g_usleep(50000);
    }

    return NULL;
}


static int catchFalseAlarm(void) {
    return 0;
}   

void setup_x11() {
    XSetWindowAttributes attribs;
    Window w;

    if (!(d = XOpenDisplay(0))) {
        fprintf (stderr, "Couldn't connect to %s\n", XDisplayName (0));
        exit (EXIT_FAILURE);
    }

    attribs.override_redirect = True;
    w = XCreateWindow(d, DefaultRootWindow (d), 800, 800, 1, 1, 0,
            CopyFromParent, InputOnly, CopyFromParent,
            CWOverrideRedirect, &attribs);
    XMapWindow(d, w);
    XSetErrorHandler((XErrorHandler )catchFalseAlarm);
    XSync (d, 0);
}

static void activate (GtkApplication *app, gpointer user_data) {
    GtkWidget *frame;
    GtkWidget *context_frame;
    GtkWidget* hbox;
    GtkWidget* vbox;

    setup_x11();

    main_window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (main_window), "Color Helper");

    g_signal_connect (main_window, "destroy", G_CALLBACK (close_window), NULL);
    g_signal_connect (main_window, "delete-event", G_CALLBACK (delete_event), NULL);

    gtk_container_set_border_width (GTK_CONTAINER (main_window), 8);

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

    context_frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (context_frame), GTK_SHADOW_IN);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(main_window), hbox);

    gtk_box_pack_start(GTK_BOX(hbox), frame, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(hbox), context_frame, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, 0, 0, 0);

    color_name_label = gtk_label_new("Cornflower blue");
    gtk_box_pack_start(GTK_BOX(vbox), color_name_label, 0, 0, 0);

    gtk_widget_set_valign(color_name_label, GTK_ALIGN_START);
    gtk_widget_set_halign(color_name_label, GTK_ALIGN_START);

    rgb_label = gtk_label_new("127, 127, 127");
    gtk_box_pack_start(GTK_BOX(vbox), rgb_label, 0, 0, 0);

    gtk_widget_set_valign(rgb_label, GTK_ALIGN_START);
    gtk_widget_set_halign(rgb_label, GTK_ALIGN_START);

    hex_label = gtk_label_new("FF FF FF");
    gtk_box_pack_start(GTK_BOX(vbox), hex_label, 0, 0, 0);

    gtk_widget_set_valign(hex_label, GTK_ALIGN_START);
    gtk_widget_set_halign(hex_label, GTK_ALIGN_START);

    color_drawing_area = gtk_drawing_area_new ();
    context_drawing_area = gtk_drawing_area_new ();

    // set a minimum size
    gtk_widget_set_size_request (color_drawing_area, 100, 100);
    gtk_widget_set_size_request (context_drawing_area, 100, 100);

    gtk_container_add (GTK_CONTAINER (frame), color_drawing_area);
    gtk_container_add (GTK_CONTAINER (context_frame), context_drawing_area);

    // signals used to handle the backing surface
    g_signal_connect (color_drawing_area, "draw",
            G_CALLBACK (draw_cb_rgb), NULL);
    g_signal_connect (color_drawing_area,"configure-event",
            G_CALLBACK (configure_event_cb_rgb), NULL);

    g_signal_connect (context_drawing_area, "draw",
            G_CALLBACK (draw_cb_context), NULL);
    g_signal_connect (context_drawing_area,"configure-event",
            G_CALLBACK (configure_event_cb_context), NULL);

    g_signal_connect (main_window, "button-press-event",
            G_CALLBACK(on_window_clicked), NULL);

    gtk_widget_show_all (main_window);
    refresh_preferences();
}

void load_preferences() {
    preferences* prefs = preferences_read();
    app_preferences = *prefs;
    free(prefs);
}

int main (int argc, char **argv) {
    g_mutex_init(&running_mutex);
    load_preferences();
    update_thread = g_thread_new(NULL, update_thread_func, (gpointer)d);
    colors = read_colors();
    GtkApplication *app;
    int status;

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_thread_join(update_thread);
    g_object_unref (app);
    free(colors);

    return status;
}

