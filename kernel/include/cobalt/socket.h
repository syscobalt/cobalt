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

#ifndef _COBALT_SOCKET_H
#define _COBALT_SOCKET_H

#include <cobalt/types.h>

struct sockaddr {
    __sa_family_t sa_family;
    __extension__ char sa_data[];
};

struct sockaddr_storage {
    __sa_family_t ss_family;
    char __data[104];
};

#define AF_UNSPEC 0
#define AF_UNIX 1

#define SOCK_CLOEXEC (1 << 0)
#define SOCK_CLOFORK (1 << 1)
#define SOCK_NONBLOCK (1 << 2)

#define _SOCK_FLAGS (SOCK_CLOEXEC | SOCK_CLOFORK | SOCK_NONBLOCK)

#define SOCK_STREAM (1 << 3)

#endif
