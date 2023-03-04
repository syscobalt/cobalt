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

#include "utils.h"
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static bool failed = false;

static void cat(const char* path) {
    int fd;
    if (strcmp(path, "-") == 0) {
        fd = 0;
    } else {
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            warn("'%s'", path);
            failed = true;
            return;
        }
    }

    while (true) {
        char buffer[4096];
        ssize_t readSize = read(fd, buffer, sizeof(buffer));
        if (readSize < 0) {
            warn("'%s'", path);
            failed = true;
            break;
        } else if (readSize == 0) {
            break;
        }

        ssize_t writtenSize = write(1, buffer, readSize);
        if (writtenSize < 0) {
            err(1, "write");
        }
    }

    if (fd != 0) {
        close(fd);
    }
}

int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "u", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "[OPTIONS] [FILE...]\n"
                    "  -u                       (ignored)\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case 'u': // ignored
            break;
        case '?':
            return 1;
        }
    }

    if (optind < argc) {
        for (int i = optind; i < argc; i++) {
            cat(argv[i]);
        }
    } else {
        cat("-");
    }

    return failed ? 1 : 0;
}
