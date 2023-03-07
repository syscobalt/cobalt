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

#include "utils.h"
#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static bool isAbsolutePath(const char* path);

int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    bool logical = true;
    int c;
    while ((c = getopt_long(argc, argv, "LP", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "[OPTIONS]\n"
                    "  -L                       print logical path\n"
                    "  -P                       print physical path\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case 'L':
            logical = true;
            break;
        case 'P':
            logical = false;
            break;
        case '?':
            return 1;
        }
    }

    if (optind < argc) {
        errx(1, "extra operand '%s'", argv[optind]);
    }

    const char* pwd = getenv("PWD");
    if (logical && pwd && isAbsolutePath(pwd)) {
        puts(pwd);
    } else {
        pwd = getcwd(NULL, 0);
        if (!pwd) err(1, "getcwd");
        puts(pwd);
    }
}

static bool isAbsolutePath(const char* path) {
    if (*path != '/') return false;
    while (*path) {
        if (path[0] == '.' && (path[1] == '/' || path[1] == '\0' ||
                (path[1] == '.' && (path[2] == '/' || path[2] == '\0')))) {
            return false;
        }

        while (*path && *path++ != '/');
    }
    return true;
}
