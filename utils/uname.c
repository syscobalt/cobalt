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

#include "utils.h"
#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/utsname.h>

enum {
    PRINT_SYSNAME = 1 << 0,
    PRINT_NODENAME = 1 << 1,
    PRINT_RELEASE = 1 << 2,
    PRINT_VERSION = 1 << 3,
    PRINT_MACHINE = 1 << 4,
    PRINT_ALL = PRINT_SYSNAME | PRINT_NODENAME | PRINT_RELEASE | PRINT_VERSION
            | PRINT_MACHINE
};

bool needSpace = false;

static void printInfo(const char* str) {
    if (needSpace) {
        putchar(' ');
    }
    fputs(str, stdout);
    needSpace = true;
}

int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "all", no_argument, 0, 'a' },
        { "kernel-name", no_argument, 0, 's' },
        { "nodename", no_argument, 0, 'n' },
        { "kernel-release", no_argument, 0, 'r' },
        { "kernel-version", no_argument, 0, 'v' },
        { "machine", no_argument, 0, 'm' },
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    int flags = 0;
    int c;
    while ((c = getopt_long(argc, argv, "amnrsv", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "[OPTIONS]\n"
                    "  -a, --all                print all information\n"
                    "  -s, --kernel-name        print operating system name\n"
                    "  -n, --nodename           print node name\n"
                    "  -r, --kernel-release     print kernel release\n"
                    "  -v, --kernel-version     print kernel version\n"
                    "  -m, --machine            print hardware architecture\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case 'a':
            flags = PRINT_ALL;
            break;
        case 's':
            flags |= PRINT_SYSNAME;
            break;
        case 'n':
            flags |= PRINT_NODENAME;
            break;
        case 'r':
            flags |= PRINT_RELEASE;
            break;
        case 'v':
            flags |= PRINT_VERSION;
            break;
        case 'm':
            flags |= PRINT_MACHINE;
            break;
        case '?':
            return 1;
        }
    }

    if (optind < argc) {
        errx(1, "extra operand '%s'", argv[optind]);
    }

    if (flags == 0) {
        flags = PRINT_SYSNAME;
    }

    struct utsname name;
    uname(&name);

    if (flags & PRINT_SYSNAME) {
        printInfo(name.sysname);
    }
    if (flags & PRINT_NODENAME) {
        printInfo(name.nodename);
    }
    if (flags & PRINT_RELEASE) {
        printInfo(name.release);
    }
    if (flags & PRINT_VERSION) {
        printInfo(name.version);
    }
    if (flags & PRINT_MACHINE) {
        printInfo(name.machine);
    }

    putchar('\n');
}
