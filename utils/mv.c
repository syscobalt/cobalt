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
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static bool move(int sourceFd, const char* sourceName, const char* sourcePath,
        int destFd, const char* destName, const char* destPath, bool prompt);

#define MV
#include "cp.c"
#include "rm.c"

int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "force", no_argument, 0, 'f' },
        { "interactive", no_argument, 0, 'i' },
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    bool prompt = false;
    int c;
    while ((c = getopt_long(argc, argv, "fi", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "[OPTIONS] SOURCE... DESTINATION\n"
                    "  -f, --force              do not prompt\n"
                    "  -i, --interactive        prompt before overwrite\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case 'f':
            prompt = false;
            break;
        case 'i':
            prompt = true;
            break;
        case '?':
            return 1;
        }
    }

    if (optind >= argc) errx(1, "missing source operand");
    if (optind == argc - 1) errx(1, "missing destination operand");

    const char* destination = argv[argc - 1];
    if (optind == argc - 2) {
        struct stat destSt;
        int statResult = stat(destination, &destSt);
        if (statResult < 0 && errno != ENOENT) {
            err(1, "stat: '%s'", destination);
        } else if (statResult < 0 || !S_ISDIR(destSt.st_mode)) {
            bool success = move(AT_FDCWD, argv[optind], argv[optind], AT_FDCWD,
                    destination, destination, prompt);
            return success ? 0 : 1;
        }
    }

    bool success = true;

    int destFd = open(destination, O_SEARCH | O_DIRECTORY);
    if (destFd < 0) err(1, "open: '%s'", destination);
    for (int i = optind; i < argc - 1; i++) {
        const char* source = argv[i];
        char* sourceCopy = strdup(source);
        if (!sourceCopy) err(1, "strdup");
        char* destName = basename(sourceCopy);
        if (strcmp(destName, "/") == 0) {
            destName = ".";
        }
        char* destPath = malloc(strlen(destination) + strlen(destName) + 2);
        if (!destPath) err(1, "malloc");
        stpcpy(stpcpy(stpcpy(destPath, destination), "/"), destName);
        success &= move(AT_FDCWD, source, source, destFd, destName, destPath,
                prompt);
        free(sourceCopy);
        free(destPath);
    }
    return success ? 0 : 1;
}

static bool move(int sourceFd, const char* sourceName, const char* sourcePath,
        int destFd, const char* destName, const char* destPath, bool prompt) {
    struct stat sourceSt, destSt;
    if (fstatat(sourceFd, sourceName, &sourceSt, AT_SYMLINK_NOFOLLOW) < 0) {
        warn("stat: '%s'", sourcePath);
        return false;
    }
    bool destExists = true;
    if (fstatat(destFd, destName, &destSt, 0) < 0) {
        if (errno != ENOENT) {
            warn("stat: '%s'", destPath);
            return false;
        }
        destExists = false;
    }

    if (destExists && prompt) {
        fprintf(stderr, "%s: overwrite '%s'? ", program_invocation_short_name,
                destPath);
        if (!getConfirmation()) return true;
    }

    if (destExists && sourceSt.st_dev == destSt.st_dev &&
            sourceSt.st_ino == destSt.st_ino) {
        warnx("'%s' and '%s' are the same file", sourcePath, destPath);
        return false;
    }

    if (renameat(sourceFd, sourceName, destFd, destName) == 0) {
        return true;
    } else if (errno != EXDEV) {
        warn("cannot move '%s' to '%s'", sourcePath, destPath);
        return false;
    }

    if (destExists && S_ISDIR(destSt.st_mode) != S_ISDIR(sourceSt.st_mode)) {
        warnx("cannot overwrite '%s' by '%s'", destPath, sourcePath);
        return false;
    }

    if (destExists) {
        if (unlinkat(destFd, destName,
                S_ISDIR(destSt.st_mode) ? AT_REMOVEDIR : 0) < 0) {
            warn("cannot unlink '%s'", destPath);
            return false;
        }
    }

    if (!copy(sourceFd, sourceName, sourcePath, destFd, destName, destPath,
            false, false, true, ATTR_MODE | ATTR_OWNER | ATTR_TIMESTAMP)) {
        return false;
    }

    return removeFile(sourcePath, false, false, true);
}
