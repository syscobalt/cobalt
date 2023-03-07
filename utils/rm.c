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
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static bool removeFile(const char* filename, bool force, bool prompt,
        bool recursive);
static bool removeRecursively(const char* dirname, bool force, bool prompt);

#ifndef MV
int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "force", no_argument, 0, 'f' },
        { "recursive", no_argument, 0, 'r' },
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    bool force = false;
    bool prompt = false;
    bool recursive = false;
    int c;
    while ((c = getopt_long(argc, argv, "fiRr", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "[OPTIONS] FILE...\n"
                    "  -f, --force              ignore nonexistent files\n"
                    "  -i                       prompt for confirmation\n"
                    "  -r, -R, --recursive      recursively remove directories\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case 'f':
            force = true;
            prompt = false;
            break;
        case 'i':
            force = false;
            prompt = true;
            break;
        case 'R':
        case 'r':
            recursive = true;
            break;
        case '?':
            return 1;
        }
    }

    if (optind >= argc) errx(1, "missing operand");

    int exitStatus = 0;
    for (int i = optind; i < argc; i++) {
        char* nameCopy = strdup(argv[i]);
        if (!nameCopy) err(1, "strdup");
        char* base = basename(nameCopy);
        if (strcmp(base, "/") == 0) {
            warnx("cannot remove root directory");
            free(nameCopy);
            exitStatus = 1;
            continue;
        }
        if (strcmp(base, ".") == 0 || strcmp(base, "..") == 0) {
            warnx("cannot remove '%s'", argv[i]);
            free(nameCopy);
            exitStatus = 1;
            continue;
        }
        free(nameCopy);
        if (!removeFile(argv[i], force, prompt, recursive)) {
            exitStatus = 1;
        }
    }
    return exitStatus;
}
#endif

static bool removeFile(const char* filename, bool force, bool prompt,
        bool recursive) {
    struct stat st;
    if (lstat(filename, &st) < 0) {
        if (force && errno == ENOENT) return true;
        warn("cannot remove '%s'", filename);
        return false;
    }

    if (S_ISDIR(st.st_mode)) {
        if (!recursive) {
            errno = EISDIR;
            warn("'%s'", filename);
            return false;
        }
        if (prompt /* || TODO: no write permission */) {
            fprintf(stderr, "%s: descend into directory '%s'? ",
                    program_invocation_short_name, filename);
            if (!getConfirmation()) return true;
        }

        bool result = removeRecursively(filename, force, prompt);

        if (prompt) {
            fprintf(stderr, "%s: remove directory '%s'? ",
                    program_invocation_short_name, filename);
            if (!getConfirmation()) return true;
        }
        if (rmdir(filename) < 0) {
            warn("cannot remove '%s'", filename);
            return false;
        }
        return result;
    } else {
        if (prompt /* || TODO: no write permission */) {
            fprintf(stderr, "%s: remove file '%s'? ",
                    program_invocation_short_name, filename);
            if (!getConfirmation()) return true;
        }
        if (unlink(filename) < 0) {
            warn("cannot remove '%s'", filename);
            return false;
        }
        return true;
    }
}

static bool removeRecursively(const char* dirname, bool force, bool prompt) {
    DIR* dir = opendir(dirname);
    if (!dir) {
        warn("fopendir: '%s'", dirname);
        return false;
    }

    size_t dirnameLength = strlen(dirname);
    bool result = true;
    errno = 0;
    struct dirent* dirent = readdir(dir);
    while (dirent) {
        if (strcmp(dirent->d_name, ".") == 0 ||
                strcmp(dirent->d_name, "..") == 0) {
            dirent = readdir(dir);
            continue;
        }
        char* name = malloc(dirnameLength + strlen(dirent->d_name) + 2);
        if (!name) err(1, "malloc");
        stpcpy(stpcpy(stpcpy(name, dirname), "/"), dirent->d_name);

        result &= removeFile(name, force, prompt, true);
        free(name);

        errno = 0;
        dirent = readdir(dir);
    }

    if (errno) {
        warn("readdir: '%s'", dirname);
        closedir(dir);
        return false;
    }
    closedir(dir);
    return result;
}
