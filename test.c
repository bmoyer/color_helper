#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include "color.c"

static Window root;

static void
query_pointer(Display *d)
{
  static int once;
  int i, x, y;
  unsigned m;
  Window w;

  if (once == 0) {
    once = 1;
    root = DefaultRootWindow(d);
  }

  if (!XQueryPointer(d, root, &root, &w, &x, &y, &i, &i, &m)) {
    for (i = -1; ++i < ScreenCount(d); ) {
      if (root == RootWindow(d, i)) {
        break;
      } 
    }
  }

  char r, g, b;
  get_color(x, y, &r, &g, &b);
  fprintf(stderr, "X: %d Y: %d\n", x, y);
}

static int
catchFalseAlarm(void)
{
  return 0;
}   

int 
main(void)
{
  XSetWindowAttributes attribs;
  Display* d;
  Window w;

  if (!(d = XOpenDisplay(0))) {
    fprintf (stderr, "Couldn't connect to %s\n", XDisplayName (0));
    exit (EXIT_FAILURE);
  }

  attribs.override_redirect = True;
  w = XCreateWindow(d, DefaultRootWindow (d), -1, -1, 1, 1, 0,
         CopyFromParent, InputOnly, CopyFromParent,
         CWOverrideRedirect, &attribs);
  XMapWindow(d, w);
  XSetErrorHandler((XErrorHandler )catchFalseAlarm);
  XSync (d, 0);

  for (;;) {
    query_pointer(d);
    usleep(1000);
  }

  return 0;
}
