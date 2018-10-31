#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include "color_detect.h"

#define N_THREADS    1
#define N_ITERATIONS 10000

GtkWidget* box;
GtkWidget* bar;
GtkWidget* label;
GtkWidget* label2;
GtkWidget* darea;
GMainContext *context;
static Window root;

Display* d;

    static int
catchFalseAlarm(void)
{
    return 0;
}   

int get_color(Display* d, int x, int y, int* r, int* b, int* g) {
    XColor c;
    XImage *image = XGetImage (d, XRootWindow (d, XDefaultScreen (d)), x, y, 1, 1, AllPlanes, XYPixmap);
    c.pixel = XGetPixel (image, 0, 0);
    XFree (image);
    XQueryColor (d, XDefaultColormap(d, XDefaultScreen (d)), &c);

    *r = (c.red/256);
    *b = (c.blue/256);
    *g = (c.green/256);
}

    static void
query_pointer(int* x, int* y)
{
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

    static gboolean
update_progress_bar(gpointer user_data)
{
    int r, g, b, x, y;
    query_pointer(&x, &y);
    char s2[20];
    get_color(d, x, y, &r, &g, &b);
    sprintf(s2, "%d, %d, %d", r, g, b);
    gtk_label_set_label(GTK_LABEL(label2),s2);
    return G_SOURCE_REMOVE;
}


    static gpointer
thread_func(gpointer user_data)
{
    //int n_thread = GPOINTER_TO_INT(user_data);
    //Display* d = (Display*)user_data;
    int n;
    GSource *source;

    //g_print("Starting thread %d\n", n_thread);

    for (;;) {
        g_usleep(1000);
        //query_pointer(d);
        source = g_idle_source_new();
        g_source_set_callback(source, update_progress_bar, NULL, NULL);
        g_source_attach(source, context);
        g_source_unref(source);
    }

    //g_print("Ending thread %d\n", n_thread);
    return NULL;
}


    gint
main(gint argc, gchar *argv[])
{
    color* colors = read_colors();
    for(int i = 0; i < NUM_COLORS; i++) {
    //for(int i = 0; i < 3; i++) {
        int r = colors[i].r;
        int g = colors[i].g;
        int b = colors[i].b;

        //printf("rgb: %d,%d,%d -- %s", colors[i].r, colors[i].g, colors[i].b, colors[i].name);
        //printf("rgb: %d,%d,%d -- ", colors[i].r, colors[i].g, colors[i].b);
        printf("%s", colors[i].name);
    }
    free(colors);
    return 0;

    // setup x11
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

    // setup gtk
    GtkWidget *window;
    GThread *thread[N_THREADS];
    int n;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated((GtkWindow*)window, 1);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    box = gtk_hbox_new(0, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    //bar = gtk_progress_bar_new();
    //gtk_box_pack_start(GTK_BOX(box), bar, 0, 0, 0);
    //gtk_container_add(GTK_CONTAINER(window), bar);

    label = gtk_label_new("i am a label");
    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 40);

    label2 = gtk_label_new("another label");
    gtk_box_pack_start(GTK_BOX(box), label2, 0, 0, 40);
    //gtk_container_add(GTK_CONTAINER(window), label);

    darea = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(box), darea, 0, 0, 40);

    context = g_main_context_default();

    for (n = 0; n < N_THREADS; ++n)
        //thread[n] = g_thread_new(NULL, thread_func, GINT_TO_POINTER(n));
        thread[n] = g_thread_new(NULL, thread_func, (gpointer)d);

    gtk_widget_show_all(window);
    gtk_main();

    for (n = 0; n < N_THREADS; ++n)
        g_thread_join(thread[n]);

    return 0;
}
