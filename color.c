#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

int get_color(int x, int y, char* r, char* b, char* g) {
    XColor c;
    Display *d = XOpenDisplay((char *) NULL);

    XImage *image;
    image = XGetImage (d, XRootWindow (d, XDefaultScreen (d)), x, y, 1, 1, AllPlanes, XYPixmap);
    c.pixel = XGetPixel (image, 0, 0);
    XFree (image);
    XQueryColor (d, XDefaultColormap(d, XDefaultScreen (d)), &c);
    //cout << c.red/256 << " " << c.green/256 << " " << c.blue/256 << "\n";
    //printf("RGB: [%d, %d, %d]\n", c.red/256, c.green/256, c.blue/256);
    *r = (char)c.red/256;
    *b = (char)c.blue/256;
    *g = (char)c.green/256;
}

/*
int main()
{
    XColor c;
    Display *d = XOpenDisplay((char *) NULL);

    int x=93;  // Pixel x 
    int y=894;  // Pixel y

    XImage *image;
    image = XGetImage (d, XRootWindow (d, XDefaultScreen (d)), x, y, 1, 1, AllPlanes, XYPixmap);
    c.pixel = XGetPixel (image, 0, 0);
    XFree (image);
    XQueryColor (d, XDefaultColormap(d, XDefaultScreen (d)), &c);
    //cout << c.red/256 << " " << c.green/256 << " " << c.blue/256 << "\n";
    printf("RGB: [%d, %d, %d]\n", c.red/256, c.green/256, c.blue/256);

    return 0;
}
*/
