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
 *  system, which can be found at https://github.com/dennis95/cobalt. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#pragma once

#include <dxui.h>

extern dxui_context* context;
extern dxui_window* compositorWindow;
extern dxui_color* lfb;
extern dxui_dim guiDim;

static const dxui_color backgroundColor = RGB(0, 0, 0);

void addDamageRect(dxui_rect rect);
void broadcastStatusEvent(void);
void composit(void);
void handleKey(dxui_control* control, dxui_key_event* event);
void handleMouse(dxui_control* control, dxui_mouse_event* event);
void handleResize(dxui_window* window, dxui_resize_event* event);
void initializeDisplay(void);
void initializeServer(void);
void pollEvents(void);