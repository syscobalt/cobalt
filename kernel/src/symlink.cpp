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

#include <stdlib.h>
#include <string.h>
#include <cobalt/kernel/symlink.h>

SymlinkVnode::SymlinkVnode(const char* target, dev_t dev)
        : Vnode(S_IFLNK | 0777, dev) {
    this->target = strdup(target);
    if (!this->target) FAIL_CONSTRUCTOR;
    stats.st_size = strlen(target);
}

SymlinkVnode::SymlinkVnode(const char* target, size_t targetLength, dev_t dev)
        : Vnode(S_IFLNK | 0777, dev) {
    this->target = strndup(target, targetLength);
    if (!this->target) FAIL_CONSTRUCTOR;
    stats.st_size = strlen(this->target);
}

SymlinkVnode::~SymlinkVnode() {
    free((char*) target);
}

char* SymlinkVnode::getLinkTarget() {
    return strdup(target);
}

ssize_t SymlinkVnode::readlink(char* buffer, size_t size) {
    size_t length = size < (size_t) stats.st_size ? size : stats.st_size;
    memcpy(buffer, target, length);
    updateTimestamps(true, false, false);
    return length;
}
