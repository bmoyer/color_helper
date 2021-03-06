# Color Helper

Simple Linux tool which identifies the color of the pixel the user is hovering over. The tool displays both the RGB/hex value of the pixel and the name of the color it matches. Directed towards individuals with colorblindness who might have trouble putting a name to a specific color on the screen.

![screenshot](res/screenshot.png "Screenshot")

Dependencies:
* GTK3.22+
* X11

To run:
```
git clone https://github.com/bmoyer/color_helper.git
mkdir color_helper/build
cd color_helper/build
cmake ..
make -j8
./color_helper
```

## Map file
Color names are stored in a map file with their RGB values. The map file is a newline-separated list where each item is of the format `Color name,R,G,B`. Simple example:

```
Red,255,0,0
Green,0,255,0
Blue,0,0,255
```

To identify a color's name from its RGB value, the tool finds the closest color from the map file. So, a larger list of colors will result in more specific color naming.

## Preferences

The preferences dialog can be accessed by right-clicking anywhere on the UI and selecting 'Preferences' from the dropdown menu. Some of the preferences supported are:
- Hide and show various elements of the UI (title bar, crosshair display, etc.)
- Custom color map file
- Frames per second
- Zoom level

By default, preferences are stored in ~/config/colorhelperrc.

