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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <cobalt/conf.h>
#include <cobalt/kernel/clock.h>
#include <cobalt/kernel/process.h>
#include <cobalt/kernel/vnode.h>

Vnode::Vnode(mode_t mode, dev_t dev) {
    stats.st_dev = dev;
    stats.st_ino = (uintptr_t) this;
    stats.st_mode = mode;
    stats.st_nlink = 0;
    stats.st_uid = 0;
    stats.st_gid = 0;
    stats.st_rdev = (uintptr_t) this;
    stats.st_size = 0;
    updateTimestamps(true, true, true);
    stats.st_blksize = 0x1000;

    mutex = KTHREAD_MUTEX_INITIALIZER;
}

Vnode::~Vnode() {
    assert(stats.st_nlink == 0);
}

static Reference<Vnode> resolvePathExceptLastComponent(
        const Reference<Vnode>& vnode, const char* path,
        size_t& symlinksFollowed, const char*& lastComponent);

static Reference<Vnode> followPath(Reference<Vnode>& vnode, const char* name,
        size_t nameLength, size_t& symlinksFollowed, bool followSymlink) {
    Reference<Vnode> currentVnode = vnode;
    Reference<Vnode> nextVnode = currentVnode->getChildNode(name, nameLength);
    if (!nextVnode) return nullptr;

    while (S_ISLNK(nextVnode->stat().st_mode) && followSymlink) {
        if (++symlinksFollowed > SYMLOOP_MAX) {
            errno = ELOOP;
            return nullptr;
        }
        char* symlinkDestination = nextVnode->getLinkTarget();
        if (!symlinkDestination) return nullptr;

        const char* lastComponent;
        currentVnode = resolvePathExceptLastComponent(currentVnode,
                symlinkDestination, symlinksFollowed, lastComponent);
        if (!currentVnode || !*lastComponent) {
            free(symlinkDestination);
            return currentVnode;
        }

        nextVnode = currentVnode->getChildNode(lastComponent,
                strcspn(lastComponent, "/"));
        free(symlinkDestination);
        if (!nextVnode) return nullptr;
    }

    return nextVnode;
}

static Reference<Vnode> resolvePathExceptLastComponent(
        const Reference<Vnode>& vnode, const char* path,
        size_t& symlinksFollowed, const char*& lastComponent) {
    Reference<Vnode> currentVnode = vnode;

    if (*path == '/') {
        currentVnode = Process::current()->rootFd->vnode;
    }

    lastComponent = path;
    while (*lastComponent == '/') {
        lastComponent++;
    }
    const char* slash = strchr(lastComponent, '/');

    while (slash) {
        size_t componentLength = slash - lastComponent;
        const char* next = slash + 1;
        while (*next == '/') {
            next++;
        }
        if (!*next) break;

        currentVnode = followPath(currentVnode, lastComponent, componentLength,
                symlinksFollowed, true);
        if (!currentVnode) return nullptr;
        currentVnode = currentVnode->resolve();

        if (!S_ISDIR(currentVnode->stat().st_mode)) {
            errno = ENOTDIR;
            return nullptr;
        }

        lastComponent = next;
        slash = strchr(lastComponent, '/');
    }

    return currentVnode;
}

Reference<Vnode> resolvePathExceptLastComponent(const Reference<Vnode>& vnode,
        const char* path, const char** lastComponent,
        bool followFinalSymlink /*= false*/) {
    size_t symlinksFollowed = 0;
    Reference<Vnode> result = resolvePathExceptLastComponent(vnode, path,
            symlinksFollowed, *lastComponent);

    while (result && followFinalSymlink) {
        Reference<Vnode> link = result->getChildNode(*lastComponent,
                strcspn(*lastComponent, "/"));
        if (!link || !S_ISLNK(link->stat().st_mode)) return result;
        if (++symlinksFollowed > SYMLOOP_MAX) {
            errno = ELOOP;
            return nullptr;
        }
        char* target = link->getLinkTarget();
        if (!target) return nullptr;
        result = resolvePathExceptLastComponent(result, target,
                symlinksFollowed, *lastComponent);
        free(target);
    }
    return result;
}

Reference<Vnode> resolvePath(const Reference<Vnode>& vnode, const char* path,
        bool followFinalSymlink /*= true*/) {
    if (!*path) {
        errno = ENOENT;
        return nullptr;
    }

    const char* lastComponent;
    size_t symlinksFollowed = 0;
    Reference<Vnode> currentVnode = resolvePathExceptLastComponent(vnode,
            path, symlinksFollowed, lastComponent);
    if (!currentVnode || !*lastComponent) {
        return currentVnode;
    }

    size_t nameLength = strcspn(lastComponent, "/");
    currentVnode = followPath(currentVnode, lastComponent, nameLength,
            symlinksFollowed, followFinalSymlink);
    if (!currentVnode) return nullptr;
    currentVnode = currentVnode->resolve();

    if (lastComponent[nameLength] && !S_ISDIR(currentVnode->stat().st_mode)) {
        errno = ENOTDIR;
        return nullptr;
    }

    return currentVnode;
}

void Vnode::updateTimestamps(bool access, bool status, bool modification) {
    struct timespec now;
    Clock::get(CLOCK_REALTIME)->getTime(&now);
    if (access) {
        stats.st_atim = now;
    }
    if (status) {
        stats.st_ctim = now;
    }
    if (modification) {
        stats.st_mtim = now;
    }
}

void Vnode::updateTimestampsLocked(bool access, bool status,
        bool modification) {
    AutoLock lock(&mutex);
    updateTimestamps(access, status, modification);
}

// Default implementation. Inheriting classes will override these functions.
Reference<Vnode> Vnode::accept(struct sockaddr* /*address*/,
        socklen_t* /*length*/, int /*fileFlags*/) {
    errno = ENOTSOCK;
    return nullptr;
}

int Vnode::bind(const struct sockaddr* /*address*/, socklen_t /*length*/,
        int /*flags*/) {
    errno = ENOTSOCK;
    return -1;
}

int Vnode::chmod(mode_t mode) {
    AutoLock lock(&mutex);
    stats.st_mode = (stats.st_mode & ~07777) | (mode & 07777);
    updateTimestamps(false, true, false);
    return 0;
}

int Vnode::chown(uid_t uid, gid_t gid) {
    AutoLock lock(&mutex);
    if (uid != (uid_t) -1) {
        stats.st_uid = uid;
    }
    if (gid != (gid_t) -1) {
        stats.st_gid = gid;
    }
    if (stats.st_mode & 0111) {
        stats.st_mode &= ~(S_ISUID | S_ISGID);
    }
    updateTimestamps(false, true, false);
    return 0;
}

int Vnode::connect(const struct sockaddr* /*address*/, socklen_t /*length*/,
        int /*flags*/) {
    errno = ENOTSOCK;
    return -1;
}

int Vnode::devctl(int /*command*/, void* restrict /*data*/, size_t /*size*/,
        int* restrict info) {
    *info = -1;
    return ENOTTY;
}

int Vnode::ftruncate(off_t /*length*/) {
    errno = EBADF;
    return -1;
}

Reference<Vnode> Vnode::getChildNode(const char* /*path*/) {
    errno = EBADF;
    return nullptr;
}

Reference<Vnode> Vnode::getChildNode(const char* /*path*/, size_t /*length*/) {
    errno = EBADF;
    return nullptr;
}

size_t Vnode::getDirectoryEntries(void** buffer, int /*flags*/) {
    *buffer = nullptr;
    errno = ENOTDIR;
    return 0;
}

char* Vnode::getLinkTarget() {
    errno = EINVAL;
    return nullptr;
}

int Vnode::isatty() {
    errno = ENOTTY;
    return 0;
}

bool Vnode::isSeekable() {
    return false;
}

int Vnode::link(const char* /*name*/, const Reference<Vnode>& /*vnode*/) {
    errno = ENOTDIR;
    return -1;
}

int Vnode::listen(int /*backlog*/) {
    errno = ENOTSOCK;
    return -1;
}

off_t Vnode::lseek(off_t /*offset*/, int /*whence*/) {
    errno = ESPIPE;
    return -1;
}

int Vnode::mkdir(const char* /*name*/, mode_t /*mode*/) {
    errno = ENOTDIR;
    return -1;
}

int Vnode::mount(FileSystem* /*filesystem*/) {
    errno = ENOTDIR;
    return -1;
}

void Vnode::onLink() {
    updateTimestamps(false, true, false);
    stats.st_nlink++;
}

bool Vnode::onUnlink(bool /*force*/) {
    updateTimestamps(false, true, false);
    stats.st_nlink--;
    return true;
}

Reference<Vnode> Vnode::open(const char* /*name*/, int /*flags*/,
        mode_t /*mode*/) {
    errno = ENOTDIR;
    return nullptr;
}

long Vnode::pathconf(int name) {
    switch (name) {
    case _PC_NAME_MAX: return -1; // unlimited
    default:
        errno = EINVAL;
        return -1;
    }
}

short Vnode::poll() {
    return 0;
}

ssize_t Vnode::pread(void* /*buffer*/, size_t /*size*/, off_t /*offset*/,
        int /*flags*/) {
    errno = ESPIPE;
    return -1;
}

ssize_t Vnode::pwrite(const void* /*buffer*/, size_t /*size*/,
        off_t /*offset*/, int /*flags*/) {
    errno = ESPIPE;
    return -1;
}

ssize_t Vnode::read(void* /*buffer*/, size_t /*size*/, int /*flags*/) {
    errno = EBADF;
    return -1;
}

ssize_t Vnode::readlink(char* /*buffer*/, size_t /*size*/) {
    errno = EINVAL;
    return -1;
}

int Vnode::rename(const Reference<Vnode>& /*oldDirectory*/,
        const char* /*oldName*/, const char* /*newName*/) {
    errno = EBADF;
    return -1;
}

Reference<Vnode> Vnode::resolve() {
    return this;
}

int Vnode::stat(struct stat* result) {
    AutoLock lock(&mutex);
    *result = stats;
    result->st_blocks = (stats.st_size + 511) / 512;
    return 0;
}

struct stat Vnode::stat() {
    struct stat result;
    stat(&result);
    return result;
}

int Vnode::symlink(const char* /*linkTarget*/, const char* /*name*/) {
    errno = ENOTDIR;
    return -1;
}

int Vnode::sync(int /*flags*/) {
    return 0;
}

int Vnode::tcgetattr(struct termios* /*result*/) {
    errno = ENOTTY;
    return -1;
}

int Vnode::tcsetattr(int /*flags*/, const struct termios* /*termio*/) {
    errno = ENOTTY;
    return -1;
}

int Vnode::unlink(const char* /*name*/, int /*flags*/) {
    errno = ENOTDIR;
    return -1;
}

int Vnode::unmount() {
    errno = ENOTDIR;
    return -1;
}

int Vnode::utimens(struct timespec atime, struct timespec mtime) {
    AutoLock lock(&mutex);

    struct timespec now;
    Clock::get(CLOCK_REALTIME)->getTime(&now);

    if (atime.tv_nsec == UTIME_NOW) {
        stats.st_atim = now;
    } else if (atime.tv_nsec != UTIME_OMIT) {
        stats.st_atim = atime;
    }

    if (mtime.tv_nsec == UTIME_NOW) {
        stats.st_mtim = now;
    } else if (mtime.tv_nsec != UTIME_OMIT) {
        stats.st_mtim = mtime;
    }

    if (atime.tv_nsec != UTIME_OMIT || mtime.tv_nsec != UTIME_OMIT) {
        stats.st_ctim = now;
    }

    return 0;
}

ssize_t Vnode::write(const void* /*buffer*/, size_t /*size*/, int /*flags*/) {
    errno = EBADF;
    return -1;
}
