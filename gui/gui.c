/* Copyright (c) 2020, 2021, 2022 Dennis Wölfing
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* gui/gui.c
 * Graphical User Interface.
 */

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "window.h"

dxui_context* context;
dxui_window* compositorWindow;
dxui_color* lfb;
dxui_dim guiDim;
static volatile sig_atomic_t winchReceived;

static void shutdown(void) {
    dxui_shutdown(context);
}

static void onSigwinch(int signo) {
    (void) signo;
    winchReceived = 1;
}

static void handleClose(dxui_window* window) {
    (void) window;
    for (struct Window* win = topWindow; win; win = win->below) {
        closeWindow(win);
    }

    exit(0);
}

static void initialize(void) {
    atexit(shutdown);
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    context = dxui_initialize(DXUI_INIT_CURSOR);
    if (!context) dxui_panic(NULL, "Failed to initialize dxui");

    bool standalone = dxui_is_standalone(context);
    if (standalone) {
        signal(SIGWINCH, onSigwinch);
    }

    dxui_rect rect;
    rect.x = -1;
    rect.y = -1;
    dxui_dim displayDim = dxui_get_display_dim(context);

    if (standalone) {
        rect.dim = displayDim;
    } else {
        rect.width = 4 * displayDim.width / 5;
        rect.height = 4 * displayDim.height / 5;
    }

    compositorWindow = dxui_create_window(context, rect, "GUI",
            DXUI_WINDOW_COMPOSITOR);
    if (!compositorWindow) dxui_panic(context, "Failed to create a window");
    dxui_set_background(compositorWindow, backgroundColor);

    dxui_set_event_handler(compositorWindow, DXUI_EVENT_MOUSE, handleMouse);
    dxui_set_event_handler(compositorWindow, DXUI_EVENT_KEY, handleKey);
    dxui_set_event_handler(compositorWindow, DXUI_EVENT_WINDOW_RESIZED,
            handleResize);
    dxui_set_event_handler(compositorWindow, DXUI_EVENT_WINDOW_CLOSE,
            handleClose);

    guiDim = rect.dim;
    lfb = dxui_get_framebuffer(compositorWindow, guiDim);
    if (!lfb) dxui_panic(context, "Failed to create window framebuffer");

    dxui_show(compositorWindow);

    initializeDisplay();
    initializeServer();
}

int main(void) {
    initialize();
    dxui_rect rect = { .pos = {0, 0}, .dim = guiDim };
    addDamageRect(rect);
    composit();

    while (true) {
        pollEvents();
        if (winchReceived) {
            winchReceived = 0;
            dxui_resize_window(compositorWindow, dxui_get_display_dim(context));
        }
        composit();
    }
}
