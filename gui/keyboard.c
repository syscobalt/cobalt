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

#include <cobalt/kbkeys.h>
#include <stdlib.h>
#include <unistd.h>
#include "connection.h"
#include "window.h"

static bool leftGuiKey;
static bool rightGuiKey;

void handleKey(dxui_control* control, dxui_key_event* event) {
    (void) control;
    struct Window* window = topWindow;

    if (event->key == KB_LGUI) {
        leftGuiKey = true;
    } else if (event->key == -KB_LGUI) {
        leftGuiKey = false;
    } else if (event->key == KB_RGUI) {
        rightGuiKey = true;
    } else if (event->key == -KB_RGUI) {
        rightGuiKey = false;
    }

    bool guiKey = leftGuiKey || rightGuiKey;

    if (guiKey && !(window && window->flags & GUI_WINDOW_COMPOSITOR)) {
        if (event->key == KB_T) {
            pid_t pid = fork();
            if (pid == 0) {
                execl("/bin/terminal", "terminal", NULL);
                _Exit(1);
            }
        } else if (event->key == KB_Q) {
            for (struct Window* win = topWindow; win; win = win->below) {
                closeWindow(win);
            }
            exit(0);
        }
    } else if (window) {
        struct gui_event_key guiEvent;
        guiEvent.window_id = window->id;
        guiEvent.key = event->key;
        guiEvent.codepoint = event->codepoint;
        sendEvent(window->connection, GUI_EVENT_KEY, sizeof(guiEvent), &guiEvent);
    }
}
