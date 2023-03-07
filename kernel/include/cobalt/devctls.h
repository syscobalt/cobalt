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

#ifndef _COBALT_DEVCTLS_H
#define _COBALT_DEVCTLS_H

#include <cobalt/devctl.h>
#include <cobalt/winsize.h>

/* Devctl numbers that are defined by default in <devctl.h> and <sys/ioctl.h>
   are defined here. More devctl numbers are defined in headers listed below. */

#define TIOCSCTTY _DEVCTL(_IOCTL_VOID, 0)
/* _IOCTL_VOID 1 and 2 are used in <cobalt/display.h>. */

/* _IOCTL_INT 0 is used in <cobalt/display.h>. */
#define TCFLSH _DEVCTL(_IOCTL_INT, 1)
/* _IOCTL_INT 2 is used in <cobalt/mouse.h>. */

#define TIOCGWINSZ _DEVCTL(_IOCTL_PTR, 0) /* (struct winsize*) */
#define TIOCGPGRP _DEVCTL(_IOCTL_PTR, 1) /* (pid_t*) */
#define TIOCSPGRP _DEVCTL(_IOCTL_PTR, 2) /* (const pid_t*) */
/* _IOCTL_PTR 3 - 6 are used in <cobalt/display.h>. */
#define TIOCGPATH _DEVCTL(_IOCTL_PTR, 7) /* (char*) */
#define TIOCSWINSZ _DEVCTL(_IOCTL_PTR, 8) /* (const struct winsize*) */

#endif
