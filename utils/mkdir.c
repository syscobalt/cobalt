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
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// TODO: Implement -m.

static void createDirectory(const char* path, bool parents);

int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "parents", no_argument, 0, 'p' },
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    bool parents = false;

    int c;
    while ((c = getopt_long(argc, argv, "p", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "[OPTIONS] DIR...\n"
                    "  -p, --parents            create parent directories\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case 'p':
            parents = true;
            break;
        case '?':
            return 1;
        }
    }

    if (optind >= argc) errx(1, "missing operand");

    for (int i = optind; i < argc; i++) {
        createDirectory(argv[i], parents);
    }
}

static void createDirectory(const char* path, bool parents) {
    if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
        if (parents && errno == EEXIST) {
            // Check whether the existing file is a directory.
            // POSIX is unclear about whether this check is actually required.
            struct stat st;
            if (stat(path, &st) < 0) err(1, "stat");
            if (!S_ISDIR(st.st_mode)) {
                errno = EEXIST;
                err(1, "'%s'", path);
            }
        } else if (parents && errno == ENOENT) {
            char* parentName = strdup(path);
            if (!parentName) err(1, "strdup");
            parentName = dirname(parentName);
            createDirectory(parentName, parents);
            free(parentName);
            createDirectory(path, parents);
        } else {
            err(1, "'%s'", path);
        }
    }
}
