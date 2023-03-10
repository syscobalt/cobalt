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

#include "connection.h"
#include "window.h"

static const int minimumWindowHeight = 100;
static const int minimumWindowWidth = 100;

static int cursor = DXUI_CURSOR_ARROW;
static bool leftClick;
static dxui_pos mousePos;
static int resizeDirection;

static void sendLeaveEvent(void) {
    struct gui_event_mouse event;
    event.window_id = mouseWindow->id;
    event.x = 0;
    event.y = 0;
    event.flags = GUI_MOUSE_LEAVE;
    sendEvent(mouseWindow->connection, GUI_EVENT_MOUSE, sizeof(event), &event);
}

void handleMouse(dxui_control* control, dxui_mouse_event* event) {
    (void) control;

    if (event->flags & DXUI_MOUSE_RELATIVE) {
        if (!topWindow || !topWindow->relativeMouse) return;

        struct gui_event_mouse guiEvent;
        guiEvent.window_id = topWindow->id;
        guiEvent.x = event->pos.x;
        guiEvent.y = event->pos.y;
        guiEvent.flags = event->flags;
        sendEvent(topWindow->connection, GUI_EVENT_MOUSE, sizeof(guiEvent),
                &guiEvent);
        return;
    }

    if (event->flags & DXUI_MOUSE_LEAVE) {
        leftClick = false;
        changingWindow = NULL;
        resizeDirection = 0;
        if (mouseWindow) {
            sendLeaveEvent();
            mouseWindow = NULL;
        }
        return;
    }

    dxui_pos oldPos = mousePos;
    mousePos = event->pos;

    int status = 0;
    struct Window* win = NULL;
    if (!(event->flags & DXUI_MOUSE_LEFT) || !changingWindow) {
        for (win = topWindow; win; win = win->below) {
            if (!win->visible) continue;
            status = checkMouseInteraction(win, mousePos);
            if (status) break;
        }
    }

    if (!leftClick) {
        int newCursor;
        if (status == RESIZE_LEFT || status == RESIZE_RIGHT) {
            newCursor = DXUI_CURSOR_RESIZE_HORIZONTAL;
        } else if (status == RESIZE_TOP || status == RESIZE_BOTTOM) {
            newCursor = DXUI_CURSOR_RESIZE_VERTICAL;
        } else if (status == RESIZE_TOP_LEFT ||
                status == RESIZE_BOTTOM_RIGHT) {
            newCursor = DXUI_CURSOR_RESIZE_DIAGONAL1;
        } else if (status == RESIZE_TOP_RIGHT ||
                status == RESIZE_BOTTOM_LEFT) {
            newCursor = DXUI_CURSOR_RESIZE_DIAGONAL2;
        } else if (status == CLIENT_AREA) {
            newCursor = win->cursor;
        } else {
            newCursor = DXUI_CURSOR_ARROW;
        }

        if (cursor != newCursor) {
            cursor = newCursor;
            dxui_set_cursor(compositorWindow, cursor);
        }

        if (event->flags & DXUI_MOUSE_LEFT) {
            leftClick = true;
        }

        if (leftClick && win) {
            moveWindowToTop(win);
            if (status == CLOSE_BUTTON) {
                struct gui_event_window_close_button msg;
                msg.window_id = win->id;
                sendEvent(win->connection, GUI_EVENT_CLOSE_BUTTON, sizeof(msg),
                        &msg);
            } else if (status == TITLE_BAR) {
                changingWindow = win;
            } else if (status != CLIENT_AREA) {
                changingWindow = win;
                resizeDirection = status;
            }
        }
    } else if (!(event->flags & DXUI_MOUSE_LEFT)) {
        leftClick = false;
        changingWindow = NULL;
        resizeDirection = 0;
    } else if (changingWindow && resizeDirection == 0) {
        addDamageRect(changingWindow->rect);
        changingWindow->rect.x += mousePos.x - oldPos.x;
        changingWindow->rect.y += mousePos.y - oldPos.y;
        addDamageRect(changingWindow->rect);
    } else if (changingWindow) {
        dxui_rect rect = changingWindow->rect;
        if (resizeDirection & RESIZE_LEFT) {
            rect.width += rect.x - mousePos.x;
            rect.x = mousePos.x;
        } else if (resizeDirection & RESIZE_RIGHT) {
            rect.width = mousePos.x - rect.x;
        }
        if (resizeDirection & RESIZE_TOP) {
            rect.height += rect.y - mousePos.y;
            rect.y = mousePos.y;
        } else if (resizeDirection & RESIZE_BOTTOM) {
            rect.height = mousePos.y - rect.y;
        }

        if (!dxui_rect_equals(rect, changingWindow->rect) &&
                rect.width >= minimumWindowWidth &&
                rect.height >= minimumWindowHeight) {
            resizeWindow(changingWindow, rect);
        }
    }

    if (win && status == CLIENT_AREA) {
        if (mouseWindow && mouseWindow != win) {
            sendLeaveEvent();
            mouseWindow = win;
        }

        struct gui_event_mouse guiEvent;
        guiEvent.window_id = win->id;
        guiEvent.x = mousePos.x - getClientRect(win).x;
        guiEvent.y = mousePos.y - getClientRect(win).y;
        guiEvent.flags = event->flags;
        sendEvent(win->connection, GUI_EVENT_MOUSE, sizeof(guiEvent),
                &guiEvent);
    } else if (mouseWindow) {
        sendLeaveEvent();
        mouseWindow = NULL;
    }
}
