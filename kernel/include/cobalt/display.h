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

#ifndef _COBALT_DISPLAY_H
#define _COBALT_DISPLAY_H

#include <stddef.h>
#include <cobalt/devctl.h>

/* Set the display mode. The new mode is returned in info. */
#define DISPLAY_SET_MODE _DEVCTL(_IOCTL_INT, 0)
/* Get the display resolution. */
#define DISPLAY_GET_RESOLUTION _DEVCTL(_IOCTL_PTR, 3)
/* Draw to the screen. Only supported in LFB mode. */
#define DISPLAY_DRAW _DEVCTL(_IOCTL_PTR, 4)
/* Get the current video mode. */
#define DISPLAY_GET_VIDEO_MODE _DEVCTL(_IOCTL_PTR, 5)
/* Set the video mode. */
#define DISPLAY_SET_VIDEO_MODE _DEVCTL(_IOCTL_PTR, 6)
/* Make the current process the display owner. */
#define DISPLAY_ACQUIRE _DEVCTL(_IOCTL_VOID, 1)
/* Stop owning the display. */
#define DISPLAY_RELEASE _DEVCTL(_IOCTL_VOID, 2)

#define DISPLAY_MODE_QUERY 0
#define DISPLAY_MODE_TEXT 1
#define DISPLAY_MODE_LFB 2

struct display_resolution {
    unsigned int width;
    unsigned int height;
};

struct display_draw {
    void* lfb;
    size_t lfb_pitch;
    unsigned int lfb_x;
    unsigned int lfb_y;
    unsigned int draw_x;
    unsigned int draw_y;
    unsigned int draw_width;
    unsigned int draw_height;
};

struct video_mode {
    unsigned int video_height;
    unsigned int video_width;
    unsigned int video_bpp; /* 0 for text mode */
};

#undef RGB
#undef RGBA
/* Supported alpha values are 0 (transparent) and 255 (not transparent). */
#define RGBA(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define RGB(r, g, b) RGBA(r, g, b, 255U)

#endif
