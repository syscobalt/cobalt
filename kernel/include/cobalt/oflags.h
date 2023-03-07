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

#ifndef _COBALT_OFLAGS_H
#define _COBALT_OFLAGS_H

#define O_EXEC (1 << 0)
#define O_RDONLY (1 << 1)
#define O_WRONLY (1 << 2)
#define O_RDWR (O_RDONLY | O_WRONLY)
#define O_SEARCH O_EXEC

#define O_ACCMODE (O_EXEC | O_RDONLY | O_WRONLY)

#define O_APPEND (1 << 3)
#define O_CLOEXEC (1 << 4)
#define O_CREAT (1 << 5)
#define O_DIRECTORY (1 << 6)
#define O_EXCL (1 << 7)
#define O_NOCTTY (1 << 8)
#define O_NOFOLLOW (1 << 9)
#define O_NONBLOCK (1 << 10)
#define O_SYNC (1 << 11)
#define O_TRUNC (1 << 12)
#define O_NOCLOBBER (1 << 13)
#define O_CLOFORK (1 << 14)
#define O_TTY_INIT 0

#endif
