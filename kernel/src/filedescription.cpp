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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <cobalt/dent.h>
#include <cobalt/fcntl.h>
#include <cobalt/seek.h>
#include <cobalt/kernel/directory.h>
#include <cobalt/kernel/file.h>
#include <cobalt/kernel/filedescription.h>

#define FILE_STATUS_FLAGS (O_APPEND | O_NONBLOCK | O_SYNC)

FileDescription::FileDescription(const Reference<Vnode>& vnode, int flags)
        : vnode(vnode) {
    mutex = KTHREAD_MUTEX_INITIALIZER;
    offset = 0;
    fileFlags = flags & (O_ACCMODE | FILE_STATUS_FLAGS);
    dents = nullptr;
    dentsSize = 0;
}

FileDescription::~FileDescription() {
    free(dents);
}

Reference<FileDescription> FileDescription::accept4(struct sockaddr* address,
        socklen_t* length, int flags) {
    Reference<Vnode> socket = vnode->accept(address, length, fileFlags);
    if (!socket) return nullptr;
    int socketFileFlags = O_RDWR;
    if (flags & SOCK_NONBLOCK) socketFileFlags |= O_NONBLOCK;
    return new FileDescription(socket, socketFileFlags);
}

int FileDescription::bind(const struct sockaddr* address, socklen_t length) {
    return vnode->bind(address, length, fileFlags);
}

int FileDescription::connect(const struct sockaddr* address, socklen_t length) {
    return vnode->connect(address, length, fileFlags);
}

int FileDescription::fcntl(int cmd, int param) {
    AutoLock lock(&mutex);

    switch (cmd) {
    case F_GETFL:
        return fileFlags;
    case F_SETFL:
        fileFlags = (param & FILE_STATUS_FLAGS) | (fileFlags & O_ACCMODE);
        return 0;
    default:
        errno = EINVAL;
        return -1;
    }
}

ssize_t FileDescription::getdents(void* buffer, size_t size, int flags) {
    if (flags & ~_DT_FLAGS) {
        errno = EINVAL;
        return -1;
    }

    AutoLock lock(&mutex);
    if (!dents) {
        dentsSize = vnode->getDirectoryEntries(&dents, flags);
        if (!dents) return -1;
    }

    size_t byteOffset = 0;
    for (off_t i = 0; i < offset; i++) {
        if (byteOffset + sizeof(struct posix_dent) > dentsSize) break;
        posix_dent* dent = (posix_dent*) ((uintptr_t) dents + byteOffset);
        byteOffset += dent->d_reclen;
    }

    void* copyBegin = (void*) ((uintptr_t) dents + byteOffset);
    // Determine the number of bytes to copy.
    size_t copySize = 0;
    if (size > SSIZE_MAX) size = SSIZE_MAX;
    while (true) {
        if (byteOffset + copySize + sizeof(struct posix_dent) > dentsSize) {
            break;
        }
        posix_dent* dent = (posix_dent*) ((uintptr_t) copyBegin + copySize);
        if (copySize + dent->d_reclen > size) break;
        copySize += dent->d_reclen;
        offset++;
    }

    if (copySize == 0) {
        if (byteOffset == dentsSize) return 0;
        errno = EINVAL;
        return -1;
    }

    memcpy(buffer, copyBegin, copySize);
    vnode->updateTimestampsLocked(true, false, false);
    return copySize;
}

off_t FileDescription::lseek(off_t offset, int whence) {
    AutoLock lock(&mutex);

    if (whence == SEEK_CUR) {
        if (__builtin_add_overflow(offset, this->offset, &offset)) {
            errno = EOVERFLOW;
            return -1;
        }
    }

    off_t result = vnode->lseek(offset, whence);
    if (result < 0) return -1;
    if (result == 0 && dents) {
        free(dents);
        dents = nullptr;
        dentsSize = 0;
    }

    this->offset = result;
    return result;
}

Reference<FileDescription> FileDescription::openat(const char* path, int flags,
        mode_t mode) {
    const char* name;
    Reference<Vnode> parentVnode = resolvePathExceptLastComponent(vnode, path,
            &name, !(flags & (O_EXCL | O_NOFOLLOW)));
    if (!parentVnode) return nullptr;
    Reference<Vnode> vnode = parentVnode;
    if (!*name) name = ".";
    vnode = parentVnode->open(name, flags, mode);
    if (!vnode) return nullptr;
    vnode = vnode->resolve();

    mode = vnode->stat().st_mode;

    if (S_ISLNK(mode)) {
        errno = ELOOP;
        return nullptr;
    }

    if (flags & O_CREAT && S_ISDIR(mode)) {
        errno = EISDIR;
        return nullptr;
    }
    if (flags & O_DIRECTORY && !S_ISDIR(mode)) {
        errno = ENOTDIR;
        return nullptr;
    }

    if (flags & O_TRUNC) {
        vnode->ftruncate(0);
    }

    return new FileDescription(vnode, flags);
}

ssize_t FileDescription::read(void* buffer, size_t size) {
    if (vnode->isSeekable()) {
        AutoLock lock(&mutex);
        ssize_t result = vnode->pread(buffer, size, offset, fileFlags);

        if (result != -1) {
            offset += result;
        }
        return result;
    }
    return vnode->read(buffer, size, fileFlags);
}

int FileDescription::tcgetattr(struct termios* result) {
    return vnode->tcgetattr(result);
}

int FileDescription::tcsetattr(int flags, const struct termios* termio) {
    return vnode->tcsetattr(flags, termio);
}

ssize_t FileDescription::write(const void* buffer, size_t size) {
    if (vnode->isSeekable()) {
        AutoLock lock(&mutex);
        ssize_t result = vnode->pwrite(buffer, size, offset, fileFlags);

        if (result != -1) {
            offset = fileFlags & O_APPEND ? vnode->stat().st_size :
                    offset + result;
        }
        return result;
    }
    return vnode->write(buffer, size, fileFlags);
}
