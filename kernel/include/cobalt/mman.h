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

#ifndef _COBALT_MMAN_H
#define _COBALT_MMAN_H

#define PROT_READ (1 << 0)
#define PROT_WRITE (1 << 1)
#define PROT_EXEC (1 << 2)
#define PROT_NONE 0

#define _PROT_FLAGS (PROT_READ | PROT_WRITE | PROT_EXEC | PROT_NONE)

#define MAP_PRIVATE (1 << 0)
#define MAP_ANONYMOUS (1 << 1)

#define MAP_FAILED ((void*) 0)

#if defined(__is_cobalt_kernel) || defined(__is_cobalt_libc)
/* The mmap() function has to many parameters to be passed in registers */
#  include <stddef.h>
#  include <cobalt/types.h>
struct __mmapRequest {
    void* _addr;
    size_t _size;
    int _protection;
    int _flags;
    int _fd;
    __off_t _offset;
};
#endif

#endif
