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

#include <stdio.h>
#include <unistd.h>

int main(void) {
    struct meminfo info;
    meminfo(&info);
    size_t used = info.mem_total - info.mem_available;
    size_t cached = info.mem_available - info.mem_free;
    printf("total:     %9zu KiB\nused:      %9zu KiB\navailable: %9zu KiB\n"
            "free:      %9zu KiB\ncached:    %9zu KiB\n",
            info.mem_total / 1024, used / 1024, info.mem_available / 1024,
            info.mem_free / 1024, cached / 1024);
}
