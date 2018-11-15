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
static cairo_surface_t *surface = NULL;
Display* d;
GThread* thread;
GtkWidget* color_name_label;
GtkWidget* rgb_label;
GtkWidget *drawing_area;
GMainContext *context;

color* colors;

static void
clear_surface (void)
{
  cairo_t *cr;

  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);

  cairo_destroy (cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean
configure_event_cb (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  if (surface)
    cairo_surface_destroy (surface);

  surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               gtk_widget_get_allocated_width 
(widget),
                                               gtk_widget_get_allocated_height 
(widget));

  /* Initialize the surface to white */
  clear_surface ();

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer   data)
{
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

/* Draw a rectangle on the surface at the given position */
static void
draw_brush (GtkWidget *widget,
            gdouble    x,
            gdouble    y)
{
  cairo_t *cr;

  /* Paint to the surface, where we store our state */
  cr = cairo_create (surface);
  cairo_set_source_rgb(cr, 0.5, 0, 0.5);

  int RECT_SIZE = 100;
  cairo_rectangle (cr, x - 3, y - 3, RECT_SIZE, RECT_SIZE);
  cairo_fill (cr);

  cairo_destroy (cr);

  /* Now invalidate the affected region of the drawing area. */
  gtk_widget_queue_draw_area (widget, x - 3, y - 3, RECT_SIZE, RECT_SIZE);
}

static void draw_rect(color c)
{
  cairo_t *cr;
  /* Paint to the surface, where we store our state */
  cr = cairo_create (surface);
  cairo_set_source_rgb(cr, c.r/255.0, c.g/255.0, c.b/255.0);

  int RECT_SIZE = 100;
  cairo_rectangle (cr, 0, 0, RECT_SIZE, RECT_SIZE);
  cairo_fill (cr);

  cairo_destroy (cr);

  /* Now invalidate the affected region of the drawing area. */
  gtk_widget_queue_draw_area (drawing_area, 0, 0, RECT_SIZE, RECT_SIZE);
}

/* Handle button press events by either drawing a rectangle
 * or clearing the surface, depending on which button was pressed.
 * The ::button-press signal handler receives a GdkEventButton
 * struct which contains this information.
 */
static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       gpointer        data)
{
  /* paranoia check, in case we haven't gotten a configure event */
  if (surface == NULL)
    return FALSE;

  if (event->button == GDK_BUTTON_PRIMARY)
    {
      draw_brush (widget, event->x, event->y);
    }
  else if (event->button == GDK_BUTTON_SECONDARY)
    {
      clear_surface ();
      gtk_widget_queue_draw (widget);
    }

  /* We've handled the event, stop processing */
  return TRUE;
}

/* Handle motion events by continuing to draw if button 1 is
 * still held down. The ::motion-notify signal handler receives
 * a GdkEventMotion struct which contains this information.
 */
static gboolean
motion_notify_event_cb (GtkWidget      *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  /* paranoia check, in case we haven't gotten a configure event */
  if (surface == NULL)
    return FALSE;

  if (event->state & GDK_BUTTON1_MASK)
    draw_brush (widget, event->x, event->y);

  /* We've handled it, stop processing */
  return TRUE;
}

static void
close_window (void)
{
  if (surface)
    cairo_surface_destroy (surface);
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

void get_color(Display* d, int x, int y, int* r, int* b, int* g) {
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

    static gboolean
update_color(gpointer user_data)
{
    int r, g, b, x, y;
    query_pointer(&x, &y);
    char s2[20];
    //get_color(d, x, y, &r, &g, &b);
    get_color(d, x, y, &r, &b, &g);
    
    // set RGB readout
    sprintf(s2, "%d, %d, %d", r, g, b);
    gtk_label_set_label(GTK_LABEL(rgb_label),s2);

    // set name readout
    int yc, uc, vc;
    yuv_from_rgb(&yc, &uc, &vc, r, g, b);
    //color c = nearest_color(yc, uc, vc, colors, MAX_COLORS);
    color c = nearest_color(r,g,b, colors, MAX_COLORS);
    gtk_label_set_label(GTK_LABEL(color_name_label), c.name);

    draw_rect(c);

    return G_SOURCE_REMOVE;
}

static gpointer thread_func(gpointer user_data)
{
    //int n_thread = GPOINTER_TO_INT(user_data);
    //Display* d = (Display*)user_data;
    GSource *source;

    //g_print("Starting thread %d\n", n_thread);

    for (;;) {
        g_usleep(5000);
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
  GtkWidget* box;

  setup_x11();

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "color helper");

  g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  gtk_container_add(GTK_CONTAINER(window), box);

  //gtk_container_add (GTK_CONTAINER (window), frame);
  gtk_box_pack_start(GTK_BOX(box), frame, 0, 0, 0);

  color_name_label = gtk_label_new("Cornflower blue");
  gtk_box_pack_start(GTK_BOX(box), color_name_label, 0, 0, 0);

  rgb_label = gtk_label_new("127, 127, 127");
  gtk_box_pack_start(GTK_BOX(box), rgb_label, 0, 0, 0);

  drawing_area = gtk_drawing_area_new ();
  /* set a minimum size */
  gtk_widget_set_size_request (drawing_area, 100, 100);

  gtk_container_add (GTK_CONTAINER (frame), drawing_area);

  /* Signals used to handle the backing surface */
  g_signal_connect (drawing_area, "draw",
                    G_CALLBACK (draw_cb), NULL);
  g_signal_connect (drawing_area,"configure-event",
                    G_CALLBACK (configure_event_cb), NULL);

  /* Event signals */
  g_signal_connect (drawing_area, "motion-notify-event",
                    G_CALLBACK (motion_notify_event_cb), NULL);
  g_signal_connect (drawing_area, "button-press-event",
                    G_CALLBACK (button_press_event_cb), NULL);

  /* Ask to receive events the drawing area doesn't normally
   * subscribe to. In particular, we need to ask for the
   * button press and motion notify events that want to handle.
   */
  gtk_widget_set_events (drawing_area, gtk_widget_get_events (drawing_area)
                                     | GDK_BUTTON_PRESS_MASK
                                     | GDK_POINTER_MOTION_MASK);

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

