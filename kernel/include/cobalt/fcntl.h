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

#ifndef _COBALT_FCNTL_H
#define _COBALT_FCNTL_H

#define AT_FDCWD (-1)

/* Flags for fstatat(2). */
#define AT_SYMLINK_NOFOLLOW (1 << 0)

/* Flags for linkat(2). */
#define AT_SYMLINK_FOLLOW (1 << 0)

/* Flags for unlinkat(2). */
#define AT_REMOVEDIR (1 << 0)
/* Non standard flag to remove files */
#define AT_REMOVEFILE (1 << 1)

#define F_DUPFD 0
#define F_DUPFD_CLOEXEC 1
#define F_GETFD 2
#define F_SETFD 3
#define F_GETFL 4
#define F_SETFL 5
#define F_DUPFD_CLOFORK 6

#define FD_CLOEXEC (1 << 0)
#define FD_CLOFORK (1 << 1)

#endif
