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
 *  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Cobalt operating
 *  system, which can be found at https://github.com/dennis95/cobalt. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include <cobalt/kernel/kernel.h>

struct InterruptContext;

NORETURN void panic(const char* file, unsigned int line, const char* func,
        const InterruptContext* context, const char* format, ...)
        PRINTF_LIKE(5, 6);

#define PANIC(...) panic(__FILE__, __LINE__, __func__, __VA_ARGS__)

#endif
