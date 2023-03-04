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

#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <tar.h>
#include <cobalt/kernel/file.h>
#include <cobalt/kernel/initrd.h>
#include <cobalt/kernel/panic.h>
#include <cobalt/kernel/symlink.h>

struct TarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
};

Reference<DirectoryVnode> Initrd::loadInitrd(vaddr_t initrd) {
    Reference<DirectoryVnode> root = xnew DirectoryVnode(nullptr, 0755, 0);
    TarHeader* header = (TarHeader*) initrd;

    while (strcmp(header->magic, TMAGIC) == 0) {
        char* path;
        if (header->prefix[0]) {
            path = (char*) malloc(strnlen(header->name, sizeof(header->name)) +
                    strnlen(header->prefix, sizeof(header->prefix)) + 2);
            if (!path) PANIC("Allocation failure");

            stpcpy(stpcpy(stpcpy(path, header->prefix), "/"), header->name);
        } else {
            path = strndup(header->name, sizeof(header->name));
            if (!path) PANIC("Allocation failure");
        }

        char* path2 = strdup(path);
        if (!path2) PANIC("Allocation failure");
        char* dirName = dirname(path);
        char* fileName = basename(path2);

        Reference<DirectoryVnode> directory =
                (Reference<DirectoryVnode>) resolvePath(root, dirName);

        if (!directory) {
            PANIC("Could not add '%s' to nonexistent directory '%s'",
                    fileName, dirName);
        }

        Reference<Vnode> newFile;
        mode_t mode = (mode_t) strtol(header->mode, nullptr, 8);
        size_t size = (size_t) strtoul(header->size, nullptr, 8);
        struct timespec mtime;
        mtime.tv_sec = (time_t) strtoll(header->mtime, nullptr, 8);
        mtime.tv_nsec = 0;

        if (header->typeflag == REGTYPE || header->typeflag == AREGTYPE) {
            newFile = xnew FileVnode(header + 1, size, mode,
                    directory->stats.st_dev);
            header += 1 + ALIGNUP(size, 512) / 512;
        } else if (header->typeflag == DIRTYPE) {
            newFile = xnew DirectoryVnode(directory, mode,
                    directory->stats.st_dev);
            header++;
        } else if (header->typeflag == SYMTYPE) {
            newFile = xnew SymlinkVnode(header->linkname,
                    sizeof(header->linkname), directory->stats.st_dev);
            header++;
        } else if (header->typeflag == LNKTYPE) {
            char* linkname = strndup(header->linkname,
                    sizeof(header->linkname));
            if (!linkname || !(newFile = resolvePath(root, linkname))) {
                PANIC("Could not create symlink '/%s'", path);
            }
            free(linkname);
            header++;
        } else {
            PANIC("Unknown typeflag '%c'", header->typeflag);
        }

        newFile->stats.st_atim = mtime;
        newFile->stats.st_mtim = mtime;

        if (directory->link(fileName, newFile) < 0) {
            PANIC("Could not link file '/%s'", path);
        }
        free(path);
        free(path2);
    }

    return root;
}
