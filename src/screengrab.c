#include "screengrab.h"
#include <sys/shm.h>
#include <X11/extensions/XShm.h>


XImage* screengrab_xlib(Display* d, int x, int y, int w, int h) {
    XImage *image = XGetImage (d, XRootWindow (d, XDefaultScreen (d)), x, y, w, h, AllPlanes, ZPixmap);
    return image;
}


XImage* screengrab_xshm(Display* d, int x, int y, int w, int h) {
  XImage *image;
  XShmSegmentInfo shminfo;
  d = XOpenDisplay(NULL);
  DefaultScreen(d);

  image = XShmCreateImage(d, 
      DefaultVisual(d,0),
      24,   
      ZPixmap, NULL, &shminfo, w, h);

  shminfo.shmid = shmget(IPC_PRIVATE,
      image->bytes_per_line * image->height,
      IPC_CREAT|777);

  shminfo.shmaddr = image->data = shmat(shminfo.shmid, 0, 0);
  shminfo.readOnly = False;

  XShmAttach(d, &shminfo);

  XShmGetImage(d,
      RootWindow(d,0),
      image,
      x,
      y,
      AllPlanes);

  return image;
}

