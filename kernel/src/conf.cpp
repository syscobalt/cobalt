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

#include <errno.h>
#include <string.h>
#include <cobalt/conf.h>
#include <cobalt/kernel/syscall.h>

#define SYSNAME "Cobalt"
#ifndef COBALT_VERSION
#  define COBALT_VERSION "unknown"
#endif
#ifdef __i386__
#  define MACHINE "i686"
#elif defined(__x86_64__)
#  define MACHINE "x86_64"
#else
#  error "Unknown architecture."
#endif

static const char* getConfstr(int name) {
    switch (name) {
    case _CS_UNAME_SYSNAME: return SYSNAME;
    case _CS_UNAME_RELEASE: return COBALT_VERSION;
    case _CS_UNAME_VERSION: return __DATE__;
    case _CS_UNAME_MACHINE: return MACHINE;
    default:
        return NULL;
    }
}

size_t Syscall::confstr(int name, char* buffer, size_t size) {
    const char* result = getConfstr(name);
    if (!result) {
        errno = EINVAL;
        return 0;
    }

    return strlcpy(buffer, result, size);
}
