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
#include <libgen.h>
#include <string.h>

int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "STRING [SUFFIX]\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case '?':
            return 1;
        }
    }

    if (optind >= argc) errx(1, "missing operand");
    if (optind + 2 < argc) errx(1, "extra operand '%s'", argv[optind + 2]);

    const char* base = basename(argv[optind]);
    size_t baseLength = strlen(base);

    if (optind + 1 < argc) {
        const char* suffix = argv[optind + 1];
        size_t suffixLength = strlen(suffix);
        if (baseLength > suffixLength) {
            size_t length = baseLength - suffixLength;
            if (strcmp(base + length, suffix) == 0) {
                baseLength = length;
            }
        }
    }

    fwrite(base, 1, baseLength, stdout);
    fputc('\n', stdout);
}
