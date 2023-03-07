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

#ifndef _COBALT_DEVCTL_H
#define _COBALT_DEVCTL_H

/* For the posix_devctl() function, data is always passed as pointer and size.
   The ioctl() funtion however can accept different types. Therefore each devctl
   number needs to encode information about the type used by ioctl(). */
#define _DEVCTL(type, number) (((type) << 29) | (number))
#define _IOCTL_TYPE(devctl) (((devctl) >> 29) & 7)

#define _IOCTL_VOID 0
#define _IOCTL_INT 1
#define _IOCTL_LONG 2
#define _IOCTL_PTR 3

#endif
