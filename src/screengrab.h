#ifndef SCREENGRAB_H
#define SCREENGRAB_H 
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

XImage* screengrab_xlib(Display* d, int x, int y, int w, int h);
XImage* screengrab_xshm(Display* d, int x, int y, int w, int h);

#endif /* SCREENGRAB_H */
