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
 *  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Cobalt operating
 *  system, which can be found at https://github.com/dennis95/cobalt. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#pragma once

#include "gui.h"

struct Window {
    struct Window* above;
    struct Window* below;
    struct Connection* connection;
    dxui_color background;
    int cursor;
    unsigned int id;
    int flags;
    dxui_rect rect;
    dxui_color* titleLfb;
    dxui_dim titleDim;
    dxui_color* lfb;
    dxui_dim clientDim;
    bool relativeMouse;
    bool visible;
};

enum {
    RESIZE_TOP = 1 << 0,
    RESIZE_RIGHT = 1 << 1,
    RESIZE_BOTTOM = 1 << 2,
    RESIZE_LEFT = 1 << 3,
    CLIENT_AREA = 1 << 4,
    CLOSE_BUTTON,
    TITLE_BAR,

    RESIZE_TOP_LEFT = RESIZE_TOP | RESIZE_LEFT,
    RESIZE_TOP_RIGHT = RESIZE_TOP | RESIZE_RIGHT,
    RESIZE_BOTTOM_LEFT = RESIZE_BOTTOM | RESIZE_LEFT,
    RESIZE_BOTTOM_RIGHT = RESIZE_BOTTOM | RESIZE_RIGHT,
};

extern struct Window* changingWindow;
extern struct Window* mouseWindow;
extern struct Window* topWindow;

struct Window* addWindow(int x, int y, int width, int height, const char* title, int flags, struct Connection* connection);
int checkMouseInteraction(struct Window* window, dxui_pos pos);
void closeWindow(struct Window* window);
dxui_rect getClientRect(struct Window* window);
void hideWindow(struct Window* window);
void moveWindowToTop(struct Window* window);
void redrawWindow(struct Window* window, int width, int height, dxui_color* lfb);
void redrawWindowPart(struct Window* window, int x, int y, int width, int height, size_t pitch, dxui_color* lfb);
dxui_color renderClientArea(struct Window* window, int x, int y);
dxui_color renderWindowDecoration(struct Window* window, int x, int y);
void resizeClientRect(struct Window* window, dxui_dim dim);
void resizeWindow(struct Window* window, dxui_rect rect);
void setWindowBackground(struct Window* window, dxui_color color);
void setWindowCursor(struct Window* window, int cursor);
void setWindowTitle(struct Window* window, const char* title);
void showWindow(struct Window* window);