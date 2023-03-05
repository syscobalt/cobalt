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
#include <errno.h>
#include <getopt.h>
#include <sys/fs.h>

int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "read-only", no_argument, 0, 'r' },
        { "read-write", no_argument, 0, 'w' },
        { "rw", no_argument, 0, 'w' },
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    bool forceWrite = false;
    int mountFlags = 0;

    int c;
    while ((c = getopt_long(argc, argv, "rw", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "[OPTIONS] FILE MOUNTPOINT\n"
                    "  -r, --read-only          mount readonly\n"
                    "  -w, --rw, --read-write   force mount as writable\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case 'r':
            forceWrite = false;
            mountFlags |= MOUNT_READONLY;
            break;
        case 'w':
            forceWrite = true;
            mountFlags &= ~MOUNT_READONLY;
            break;
        case '?':
            return 1;
        }
    }

    if (optind >= argc) errx(1, "missing file operand");
    if (optind == argc - 1) errx(1, "missing mountpoint operand");
    const char* file = argv[optind];
    const char* mountPoint = argv[optind + 1];

    if (mount(file, mountPoint, "ext234", mountFlags) < 0) {
        if (!forceWrite && errno == EROFS && !(mountFlags & MOUNT_READONLY)) {
            mountFlags |= MOUNT_READONLY;
            if (mount(file, mountPoint, "ext234", mountFlags) < 0) {
                err(1, "failed to mount '%s'", file);
            }
            warnx("'%s' is not writable, mounted readonly", file);
        } else {
            err(1, "failed to mount '%s'", file);
        }
    }
}
