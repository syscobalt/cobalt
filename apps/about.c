/***
 *                                                                                      
 *      ,ad8888ba,    ,ad8888ba,    88888888ba         db         88      888888888888  
 *     d8"'    `"8b  d8"'    `"8b   88      "8b       d88b        88           88       
 *    d8'           d8'        `8b  88      ,8P      d8'`8b       88           88       
 *    88            88          88  88aaaaaa8P'     d8'  `8b      88           88       
 *    88            88          88  88""""""8b,    d8YaaaaY8b     88           88       
 *    Y8,           Y8,        ,8P  88      `8b   d8""""""""8b    88           88       
 *     Y8a.    .a8P  Y8a.    .a8P   88      a8P  d8'        `8b   88           88       
 *      `"Y8888Y"'    `"Y8888Y"'    88888888P"  d8'          `8b  88888888888  88       
 *  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Dennix operating
 *  system, which can be found at https://github.com/dennis95/dennix. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#include <dxui.h>
#include <inttypes.h>
#include <stdlib.h>

static dxui_context* context;
static dxui_label* about_label;
static dxui_label* about_label1;
static dxui_label* about_label2;

static bool createWindow(void);
static void onButtonClick(dxui_control* control, dxui_mouse_event* event);
static void onExitButtonClick(dxui_control* control, dxui_mouse_event* event);
static void onKey(dxui_control* control, dxui_key_event* event);
static void shutdown(void);

/*
 *  This is the main function, which will see if it can communicate with libdxui, to initialize a mouse
 *  cursor (if it isn't already initialized). Then, it creates a window (line 48).
 */
int main(void) {
    atexit(shutdown);
    context = dxui_initialize(DXUI_INIT_CURSOR);
    if (!context) dxui_panic(NULL, "Failed to initialize dxui.");

    if (!createWindow()) {
        dxui_panic(context, "Failed to create about window.");
    }

    dxui_pump_events(context, DXUI_PUMP_WHILE_WINDOWS_EXIST, -1);
}

/*
 *  Create the About window. This window will show the build date of the Cobalt operating system, and state
 *  that Cobalt is a UNIX-like operating system.
 */
static bool createWindow(void) {
    /*
     *  Create an instance of dxui_rect, which can be used to store rectangle size data. This size is set to what the
     *  size of the window should be. When a window is created, `rect` is passed to set the size of the newly created
     *  window to the size stored in `rect` (obviously). Then, the window is called `About`. A rule is also passed to
     *  the window creation function, which tells libdxui to make the window non-resizable.
     */
    dxui_rect rect = {{ -1, -1, 400, 200 }};
    dxui_window* window = dxui_create_window(context, rect, "About", DXUI_WINDOW_NO_RESIZE);
    /*
     *  If the window doesn't exist even after being created, it means something went wrong, and we should just quit
     *  the program immediately.
     */
    if (!window) return false;

    /*
     *  Let's reuse `rect`, to create a size for a label that says the first part of a complete sentence which we want
     *  printed; "The Cobalt operating system, compiled".
     */
    rect = (dxui_rect) {{ 10, 10, 390, 25 }};
    about_label = dxui_create_label(rect, "The Cobalt operating system, compiled");
    if (!about_label) return false;
    dxui_set_background(about_label, COLOR_WHITE);
    /*
     *  Let's now add this newly created label to the window, so that it can be renderred successfully.
     */
    dxui_add_control(window, about_label);

    /*
     *  Let's create another label, and put it under the first one, to complete the sentence "The Cobalt operating
     *  system, compiled on <DATE> at <TIME>".
     */
    rect = (dxui_rect) {{ 10, 35, 390, 50}};
    #define FINISHED_STRING "on " __DATE__ " at " __TIME__
    about_label1 = dxui_create_label(rect, FINISHED_STRING);
    if (!about_label1) return false;
    dxui_set_background(about_label1, COLOR_WHITE);
    /*
     *  Let's add the second label to the window.
     */
    dxui_add_control(window, about_label1);

    /*
     *  Let's now create a label a little more below, which contains the link to the Cobalt operating system's GitHub
     *  page.
     */
    rect = (dxui_rect) {{ 10, 85, 390, 100}};
    about_label2 = dxui_create_label(rect, "https://github.com/syscobalt/cobalt");
    if (!about_label2) return false;
    dxui_set_background(about_label2, COLOR_WHITE);
    dxui_add_control(window, about_label2);

    if (dxui_is_standalone(context)) {
        // The exit button is only needed when running without the compositor.
        rect = (dxui_rect) {{ 11, 160, 200, 25 }};
        dxui_button* exit_button = dxui_create_button(rect, "Exit");
        if (!exit_button) return false;
        dxui_set_event_handler(exit_button, DXUI_EVENT_MOUSE_CLICK, onExitButtonClick);
        dxui_add_control(window, exit_button);
    }

    dxui_set_event_handler(window, DXUI_EVENT_KEY, onKey);
    dxui_show(window);
    return true;
}

static void onButtonClick(dxui_control* control, dxui_mouse_event* event) {
    (void) event;
}

static void onExitButtonClick(dxui_control* control, dxui_mouse_event* event) {
    (void) control; (void) event;
    exit(0);
}

static void onKey(dxui_control* control, dxui_key_event* event) {
    (void) control;
}

static void shutdown(void) {
    dxui_shutdown(context);
}
