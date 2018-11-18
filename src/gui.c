#include <gtk/gtk.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include "color_detect.h"

#define MAX_COLORS   1000

/* Surface to store current scribbles */
static Window root;
static cairo_surface_t *rgb_surface = NULL;
static cairo_surface_t *context_surface = NULL;
Display* d;
GThread* thread;
GtkWidget* color_name_label;
GtkWidget* rgb_label;
GtkWidget *color_drawing_area;
GtkWidget *context_drawing_area;
GMainContext *context;

color* colors;

int CONTEXT_SIZE = 25;

static void
clear_surface (cairo_surface_t* surface)
{
  cairo_t *cr;

  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);

  cairo_destroy (cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean
configure_event_cb_rgb (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  //cairo_surface_t* surface = rgb_surface;
  if (rgb_surface)
    cairo_surface_destroy (rgb_surface);

  rgb_surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
  //surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               gtk_widget_get_allocated_width 
(widget),
                                               gtk_widget_get_allocated_height 
(widget));

  /* Initialize the surface to white */
  clear_surface (rgb_surface);

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

static gboolean
configure_event_cb_context (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  if (context_surface)
    cairo_surface_destroy (context_surface);

  context_surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
  //surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               gtk_widget_get_allocated_width 
(widget),
                                               gtk_widget_get_allocated_height 
(widget));

  /* Initialize the surface to white */
  clear_surface (context_surface);

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean
draw_cb_rgb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer   data)
{
  cairo_set_source_surface (cr, rgb_surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

static gboolean
draw_cb_context (GtkWidget *widget,
         cairo_t   *cr,
         gpointer   data)
{
  cairo_set_source_surface (cr, context_surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

static void draw_rect(int r, int g, int b)
{
  cairo_t *cr;
  /* Paint to the surface, where we store our state */
  cr = cairo_create (rgb_surface);
  cairo_set_source_rgb(cr, r/255.0, g/255.0, b/255.0);

  int RECT_SIZE = 100;
  cairo_rectangle (cr, 0, 0, RECT_SIZE, RECT_SIZE);
  cairo_fill (cr);

  cairo_destroy (cr);

  /* Now invalidate the affected region of the drawing area. */
  gtk_widget_queue_draw_area (color_drawing_area, 0, 0, RECT_SIZE, RECT_SIZE);
}

static void draw_context_pixels(color context_pixels[CONTEXT_SIZE][CONTEXT_SIZE])
{
  cairo_t *cr;
  /* Paint to the surface, where we store our state */
  cr = cairo_create (context_surface);

  int RECT_SIZE = 100;
  int PIXEL_SCALE = RECT_SIZE/CONTEXT_SIZE;

  for(int x = 0; x < CONTEXT_SIZE; x++) {
      for(int y = 0; y < CONTEXT_SIZE; y++) {
          cairo_set_source_rgb(cr,
                               context_pixels[x][y].r/255.0, 
                               context_pixels[x][y].g/255.0, 
                               context_pixels[x][y].b/255.0);
          cairo_rectangle(cr, x*PIXEL_SCALE, y*PIXEL_SCALE, PIXEL_SCALE, PIXEL_SCALE);
          cairo_fill(cr);
      }
  }


  cairo_destroy (cr);

  /* Now invalidate the affected region of the drawing area. */
  gtk_widget_queue_draw_area (context_drawing_area, 0, 0, RECT_SIZE, RECT_SIZE);
}

static void
close_window (void)
{
  if (rgb_surface)
    cairo_surface_destroy (rgb_surface);

  if (context_surface)
    cairo_surface_destroy (context_surface);
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

void get_color(Display* d, int x, int y, int* r, int* g, int* b) {
    XColor c;
    XImage *image = XGetImage (d, XRootWindow (d, XDefaultScreen (d)), x, y, 1, 1, AllPlanes, XYPixmap);
    c.pixel = XGetPixel (image, 0, 0);
    XFree (image);
    XQueryColor (d, XDefaultColormap(d, XDefaultScreen (d)), &c);

    *r = (c.red/256);
    *b = (c.blue/256);
    *g = (c.green/256);
    //printf("RGB was: (%d,%d,%d)", *r,*g,*b);
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

void get_context_pixels(Display* d, int x, int y, color    colors[CONTEXT_SIZE][CONTEXT_SIZE])
{
    //XColor c;
    //XImage *image = XGetImage (d, XRootWindow (d, XDefaultScreen (d)), x, y, CONTEXT_SIZE, CONTEXT_SIZE, AllPlanes, XYPixmap);

    // need to go from CURSOR-CONTEXT_SIZE/2 to CURSOR + CONTEXT_SIZE/2 - not from 0.
    for (int i = 0; i < CONTEXT_SIZE; i++) {
        for(int j = 0; j < CONTEXT_SIZE; j++) {
            int mouse_x = x - i + CONTEXT_SIZE/2;
            int mouse_y = y - j + CONTEXT_SIZE/2;
            clip_coords_to_display_size(&mouse_x, &mouse_y);
            get_color(d, mouse_x, mouse_y, &colors[CONTEXT_SIZE-1-i][CONTEXT_SIZE-1-j].r, &colors[CONTEXT_SIZE-1-i][CONTEXT_SIZE-1-j].g, &colors[CONTEXT_SIZE-1-i][CONTEXT_SIZE-1-j].b);
        }
    }
    //XFree (image);
    //XQueryColor (d, XDefaultColormap(d, XDefaultScreen (d)), &c);

}

    static gboolean
update_color(gpointer user_data)
{

    int r, g, b, x, y;
    query_pointer(&x, &y);
    char s2[20];

    // get color of pixel under cursor
    get_color(d, x, y, &r, &g, &b);

    // get pixels around cursor
    color context_pixels[CONTEXT_SIZE][CONTEXT_SIZE];
    get_context_pixels(d, x, y, context_pixels);
    draw_context_pixels(context_pixels);

    // set RGB readout
    sprintf(s2, "%d, %d, %d", r, g, b);
    gtk_label_set_label(GTK_LABEL(rgb_label),s2);

    // set name readout
    color c = nearest_color(r, g, b, colors, MAX_COLORS);

    gtk_label_set_label(GTK_LABEL(color_name_label), c.name);
    draw_rect(r, g, b);

    return G_SOURCE_REMOVE;
}

static gpointer thread_func(gpointer user_data)
{
    //int n_thread = GPOINTER_TO_INT(user_data);
    //Display* d = (Display*)user_data;
    GSource *source;

    //g_print("Starting thread %d\n", n_thread);

    for (;;) {
        g_usleep(50000);
        //query_pointer(d);
        source = g_idle_source_new();
        g_source_set_callback(source, update_color, NULL, NULL);
        g_source_attach(source, context);
        g_source_unref(source);
    }

    //g_print("Ending thread %d\n", n_thread);
    return NULL;
}


static int catchFalseAlarm(void)
{
    return 0;
}   

void setup_x11()
{
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

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *context_frame;
  GtkWidget* box;

  setup_x11();

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "color helper");

  g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  context_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (context_frame), GTK_SHADOW_IN);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  gtk_container_add(GTK_CONTAINER(window), box);

  gtk_box_pack_start(GTK_BOX(box), frame, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(box), context_frame, 0, 0, 0);

  color_name_label = gtk_label_new("Cornflower blue");
  gtk_box_pack_start(GTK_BOX(box), color_name_label, 0, 0, 0);

  rgb_label = gtk_label_new("127, 127, 127");
  gtk_box_pack_start(GTK_BOX(box), rgb_label, 0, 0, 0);

  color_drawing_area = gtk_drawing_area_new ();
  context_drawing_area = gtk_drawing_area_new ();
  /* set a minimum size */
  gtk_widget_set_size_request (color_drawing_area, 100, 100);
  gtk_widget_set_size_request (context_drawing_area, 100, 100);

  gtk_container_add (GTK_CONTAINER (frame), color_drawing_area);
  gtk_container_add (GTK_CONTAINER (context_frame), context_drawing_area);

  /* Signals used to handle the backing surface */
  g_signal_connect (color_drawing_area, "draw",
                    G_CALLBACK (draw_cb_rgb), NULL);
  g_signal_connect (color_drawing_area,"configure-event",
                    G_CALLBACK (configure_event_cb_rgb), NULL);

  g_signal_connect (context_drawing_area, "draw",
                    G_CALLBACK (draw_cb_context), NULL);
  g_signal_connect (context_drawing_area,"configure-event",
                    G_CALLBACK (configure_event_cb_context), NULL);


  // start update thread
  thread = g_thread_new(NULL, thread_func, (gpointer)d);

  gtk_widget_show_all (window);
}

int
main (int    argc,
      char **argv)
{
  colors = read_colors();
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_thread_join(thread);
  g_object_unref (app);

  return status;
}

