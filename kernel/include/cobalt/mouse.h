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

#ifndef _COBALT_MOUSE_H
#define _COBALT_MOUSE_H

#include <stdint.h>
#include <cobalt/devctl.h>

#define MOUSE_LEFT (1 << 0)
#define MOUSE_RIGHT (1 << 1)
#define MOUSE_MIDDLE (1 << 2)
#define MOUSE_SCROLL_UP (1 << 3)
#define MOUSE_SCROLL_DOWN (1 << 4)
#define MOUSE_ABSOLUTE (1 << 5)
#define MOUSE_NO_BUTTON_INFO (1 << 6)

#define MOUSE_SET_ABSOLUTE _DEVCTL(_IOCTL_INT, 2)

struct mouse_data {
    int16_t mouse_x;
    int16_t mouse_y;
    uint8_t mouse_flags;
};

#endif
