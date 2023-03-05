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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <cobalt/poll.h>
#include <cobalt/seek.h>
#include <cobalt/stat.h>
#include <cobalt/kernel/file.h>

FileVnode::FileVnode(const void* data, size_t size, mode_t mode, dev_t dev)
        : Vnode(S_IFREG | mode, dev) {
    this->data = (char*) malloc(size);
    if (!this->data) FAIL_CONSTRUCTOR;
    memcpy(this->data, data, size);
    stats.st_size = size;
}

FileVnode::~FileVnode() {
    free(data);
}

int FileVnode::ftruncate(off_t length) {
    if (length < 0) {
        errno = EINVAL;
        return -1;
    }
    if (sizeof(off_t) > sizeof(size_t) && length > (off_t) SIZE_MAX) {
        errno = EFBIG;
        return -1;
    }

    AutoLock lock(&mutex);
    void* newData = realloc(data, (size_t) length);
    if (!newData) {
        errno = ENOSPC;
        return -1;
    }
    data = (char*) newData;

    if (length > stats.st_size) {
        memset(data + stats.st_size, '\0', length - stats.st_size);
    }

    stats.st_size = length;
    updateTimestamps(false, true, true);
    return 0;
}

bool FileVnode::isSeekable() {
    return true;
}

off_t FileVnode::lseek(off_t offset, int whence) {
    AutoLock lock(&mutex);
    off_t base;

    if (whence == SEEK_SET || whence == SEEK_CUR) {
        base = 0;
    } else if (whence == SEEK_END) {
        base = stats.st_size;
    } else {
        errno = EINVAL;
        return -1;
    }

    off_t result;
    if (__builtin_add_overflow(base, offset, &result) || result < 0) {
        errno = EINVAL;
        return -1;
    }

    return result;
}

short FileVnode::poll() {
    return POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM;
}

ssize_t FileVnode::pread(void* buffer, size_t size, off_t offset,
        int /*flags*/) {
    if (offset < 0) {
        errno = EINVAL;
        return -1;
    }
    if (size == 0) return 0;

    AutoLock lock(&mutex);
    char* buf = (char*) buffer;

    for (size_t i = 0; i < size; i++) {
        off_t j;
        if (__builtin_add_overflow(offset, i, &j) || j >= stats.st_size) {
            return i;
        }
        buf[i] = data[j];
    }

    updateTimestamps(true, false, false);
    return size;
}

ssize_t FileVnode::pwrite(const void* buffer, size_t size, off_t offset,
        int flags) {
    if (size == 0) return 0;

    AutoLock lock(&mutex);
    if (flags & O_APPEND) {
        offset = stats.st_size;
    }
    assert(offset >= 0);

    off_t newSize;
    if (__builtin_add_overflow(offset, size, &newSize)) {
        errno = ENOSPC;
        return -1;
    }

    if (sizeof(off_t) > sizeof(size_t) && newSize > (off_t) SIZE_MAX) {
        errno = EFBIG;
        return -1;
    }

    if (newSize > stats.st_size) {
        void* newData = realloc(data, (size_t) newSize);
        if (!newData) {
            errno = ENOSPC;
            return -1;
        }
        data = (char*) newData;

        if (offset > stats.st_size) {
            // When writing after the old EOF, fill the gap with zeros.
            memset(data + stats.st_size, '\0', offset - stats.st_size);
        }

        stats.st_size = newSize;
    }

    memcpy(data + offset, buffer, size);
    updateTimestamps(false, true, true);
    return size;
}
