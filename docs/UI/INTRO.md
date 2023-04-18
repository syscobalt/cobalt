# Introduction (Deprecated)
You may be able to port X11 or Wayland to Cobalt, but you can also use the native GUI system, which has been retained when Cobalt was forked from Dennix. This subdirectory will try to document Dennix's GUI system and libdxui.
First of all, all graphical applications that are to run on Cobalt/Dennix's native GUI system will most likely require libdxui, which provides GUI-related support. Although the native GUI system isn't needed to run GUI applications that use libdxui (they can run regularly on the framebuffer), this documentation will assume you're developing with libdxui for the native GUI system.
## Getting started
To start developing GUI applications using libdxui, simply link with `-ldxui`, and use this include:
```c
#include <dxui.h>
```
After that, you can create the regular `main()` function, with some extra code above the `main()` function:
```c
#include <dxui.h>

static dxui_context* context;

int main()
{
    
}
```
Now, let's initialize dxui:
```c
#include <dxui.h>

static dxui_context* context;

int main()
{
    context = dxui_initialize(DXUI_INIT_CURSOR);
    if (!context) dxui_panic(NULL, "Failed to initialize dxui.");
}
```
The second line in the `main()` function is just to stop the program and error out saying that dxui failed to initialize, but this shouldn't normally happen.
Now, we can create and call a function to actually create the window:
```c
#include <dxui.h>

static dxui_context* context;

int main()
{
    context = dxui_initialize(DXUI_INIT_CURSOR);
    if (!context) dxui_panic(NULL, "Failed to initialize dxui.");

    if (!createWindow()) {
        dxui_panic(context, "Failed to create about window.");
    }
}

static bool createWindow(void) {
    dxui_rect rect = {{ -1, -1, 400, 200 }};
    dxui_window* window = dxui_create_window(context, rect, "My first Cobalt GUI application!", DXUI_WINDOW_NO_RESIZE);
}
```
You might wonder what the first line in the `createWindow()` function does. This line creates a rectangle-shape data type, which allows to provide the size & position where for our object. For example, here we make our window 400 pixels wide, and 200 pixels tall.
Now, you have created your very own GUI application! What you put inside the window now, is up to you.
## Doing more stuff
Currently, [Dashbloxx](https://github.com/orgs/syscobalt/people/Dashbloxx) is writing some documentation for GUI development. Here are the existing pages that document more of the GUI-related stuff:
* [LABEL.md](LABEL.md)