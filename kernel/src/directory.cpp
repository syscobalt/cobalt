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

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <cobalt/fcntl.h>
#include <cobalt/seek.h>
#include <cobalt/kernel/directory.h>
#include <cobalt/kernel/file.h>
#include <cobalt/kernel/filesystem.h>
#include <cobalt/kernel/symlink.h>

static kthread_mutex_t renameMutex = KTHREAD_MUTEX_INITIALIZER;

DirectoryVnode::DirectoryVnode(const Reference<DirectoryVnode>& parent,
        mode_t mode, dev_t dev) : Vnode(S_IFDIR | mode, dev), parent(parent) {
    childCount = 0;
    childNodes = nullptr;
    fileNames = nullptr;
    // st_nlink must also count the . and .. entries.
    stats.st_nlink += parent ? 1 : 2;
    mounted = nullptr;
}

DirectoryVnode::~DirectoryVnode() {
    free(childNodes);
    free(fileNames);
    stats.st_nlink -= parent ? 1 : 2;
}

int DirectoryVnode::link(const char* name, const Reference<Vnode>& vnode) {
    AutoLock lock(&mutex);
    AutoLock lock2(&vnode->mutex);
    return linkUnlocked(name, strlen(name), vnode);
}

int DirectoryVnode::linkUnlocked(const char* name, size_t length,
        const Reference<Vnode>& vnode) {
    if (vnode->stats.st_dev != stats.st_dev) {
        errno = EXDEV;
        return -1;
    }

    if (getChildNodeUnlocked(name, length)) {
        errno = EEXIST;
        return -1;
    }

    Reference<Vnode>* newChildNodes = (Reference<Vnode>*)
            reallocarray(childNodes, childCount + 1, sizeof(Reference<Vnode>));
    if (!newChildNodes) return -1;
    childNodes = newChildNodes;

    char** newFileNames = (char**) reallocarray(fileNames, childCount + 1,
            sizeof(const char*));
    if (!newFileNames) return -1;
    fileNames = newFileNames;

    fileNames[childCount] = strndup(name, length);
    if (!fileNames[childCount]) return -1;

    // We must use placement new here because the memory returned by realloc
    // is uninitialized so we cannot call operator=.
    new (&childNodes[childCount]) Reference<Vnode>(vnode);
    childCount++;

    vnode->onLink();
    if (S_ISDIR(vnode->stats.st_mode)) {
        stats.st_nlink++;
    }

    updateTimestamps(false, true, true);
    return 0;
}

Reference<Vnode> DirectoryVnode::getChildNode(const char* name) {
    return getChildNode(name, strlen(name));
}

Reference<Vnode> DirectoryVnode::getChildNode(const char* name, size_t length) {
    AutoLock lock(&mutex);
    return getChildNodeUnlocked(name, length);
}

Reference<Vnode> DirectoryVnode::getChildNodeUnlocked(const char* name,
        size_t length) {
    if (length == 1 && strncmp(name, ".", 1) == 0) {
        return this;
    } else if (length == 2 && strncmp(name, "..", 2) == 0) {
        return parent ? parent : this;
    }

    for (size_t i = 0; i < childCount; i++) {
        if (strncmp(name, fileNames[i], length) == 0 &&
                fileNames[i][length] == '\0') {
            return childNodes[i];
        }
    }

    errno = ENOENT;
    return nullptr;
}

int DirectoryVnode::mkdir(const char* name, mode_t mode) {
    AutoLock lock(&mutex);

    Reference<DirectoryVnode> newDirectory = new DirectoryVnode(this, mode,
            stats.st_dev);
    if (!newDirectory) return -1;
    if (linkUnlocked(name, strcspn(name, "/"), newDirectory) < 0) return -1;
    return 0;
}

size_t DirectoryVnode::getDirectoryEntries(void** buffer, int /*flags*/) {
    AutoLock lock(&mutex);

    size_t size = ALIGNUP(offsetof(struct posix_dent, d_name) + 2, // .
            alignof(struct posix_dent)) +
            ALIGNUP(offsetof(struct posix_dent, d_name) + 3, // ..
            alignof(struct posix_dent));

    for (size_t i = 0; i < childCount; i++) {
        size += ALIGNUP(offsetof(struct posix_dent, d_name) +
                strlen(fileNames[i]) + 1, alignof(struct posix_dent));
    }
    *buffer = malloc(size);
    if (!*buffer) return 0;

    void* p = *buffer;

    for (size_t i = 0; i < childCount + 2; i++) {
        struct stat st;
        const char* name;
        if (i == 0) {
            st = stats;
            name = ".";
        } else if (i == 1) {
            st = parent ? parent->stats : stats;
            name = "..";
        } else {
            st = childNodes[i - 2]->resolve()->stat();
            name = fileNames[i - 2];
        }

        posix_dent* dent = (posix_dent*) p;
        dent->d_ino = st.st_ino;
        dent->d_reclen = ALIGNUP(offsetof(struct posix_dent, d_name) +
                strlen(name) + 1, alignof(struct posix_dent));
        dent->d_type = IFTODT(st.st_mode);
        strcpy(dent->d_name, name);

        p = (void*) ((uintptr_t) p + dent->d_reclen);
    }

    return size;
}

bool DirectoryVnode::isAncestor(const Reference<Vnode>& vnode) {
    // renameMutex must be locked.
    Reference<DirectoryVnode> dir = this;
    while (dir) {
        if (dir == vnode) {
            return true;
        }
        dir = dir->parent;
    }
    return false;
}

off_t DirectoryVnode::lseek(off_t offset, int whence) {
    AutoLock lock(&mutex);
    off_t base;

    if (whence == SEEK_SET || whence == SEEK_CUR) {
        base = 0;
    } else if (whence == SEEK_END) {
        base = childCount;
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

int DirectoryVnode::mount(FileSystem* filesystem) {
    AutoLock lock(&mutex);

    if (mounted) {
        errno = EBUSY;
        return -1;
    }

    mounted = filesystem;
    return 0;
}

bool DirectoryVnode::onUnlink(bool force) {
    if (!force && mounted) {
        errno = EBUSY;
        return false;
    }

    if (!force && childCount > 0) {
        errno = ENOTEMPTY;
        return false;
    }

    updateTimestamps(false, true, false);
    stats.st_nlink--;
    return true;
}

Reference<Vnode> DirectoryVnode::open(const char* name, int flags,
        mode_t mode) {
    AutoLock lock(&mutex);

    size_t length = strcspn(name, "/");
    Reference<Vnode> vnode = getChildNodeUnlocked(name, length);
    if (!vnode) {
        if (!(flags & O_CREAT)) return nullptr;
        vnode = new FileVnode(nullptr, 0, mode & 07777, stats.st_dev);
        if (!vnode || linkUnlocked(name, length, vnode) < 0) {
            return nullptr;
        }
    } else {
        if (flags & O_EXCL) {
            errno = EEXIST;
            return nullptr;
        } else if (flags & O_NOCLOBBER && S_ISREG(vnode->stats.st_mode)) {
            errno = EEXIST;
            return nullptr;
        }
    }

    return vnode;
}

int DirectoryVnode::rename(const Reference<Vnode>& oldDirectory,
        const char* oldName, const char* newName) {
    // The rename operation requires complicated locking. Normally to avoid
    // deadlocks when we need to lock multiple vnodes we make sure to always
    // lock the containing dir before locking the child node. However for
    // renaming this is more difficult. We need to lock up to two directories
    // and the vnode being renamed. Also we cannot easily determine whether one
    // directory is an ancestor of the other before locking because a concurrent
    // rename could change the result. Therefore we must not allow more than one
    // rename at a time.
    AutoLock renameLock(&renameMutex);

    Reference<DirectoryVnode> oldDir = (Reference<DirectoryVnode>) oldDirectory;

    kthread_mutex_t* mutex1;
    kthread_mutex_t* mutex2;

    if (oldDir == this) {
        mutex1 = &mutex;
        mutex2 = nullptr;
    } else if (stats.st_ino < oldDir->stats.st_ino) {
        if (isAncestor(oldDir)) {
            mutex1 = &oldDir->mutex;
            mutex2 = &mutex;
        } else {
            mutex1 = &mutex;
            mutex2 = &oldDir->mutex;
        }
    } else {
        if (oldDir->isAncestor(this)) {
            mutex1 = &mutex;
            mutex2 = &oldDir->mutex;
        } else {
            mutex1 = &oldDir->mutex;
            mutex2 = &mutex;
        }
    }

    AutoLock lock1(mutex1);
    AutoLock lock2(mutex2);

    Reference<Vnode> vnode = oldDir->getChildNodeUnlocked(oldName,
            strcspn(oldName, "/"));
    if (!vnode) return -1;

    if (isAncestor(vnode)) {
        errno = EINVAL;
        return -1;
    }

    size_t newNameLength = strcspn(newName, "/");
    Reference<Vnode> vnode2 = getChildNodeUnlocked(newName, newNameLength);
    if (vnode == vnode2) return 0;

    {
        AutoLock vnodeLock(&vnode->mutex);
        struct stat vnodeStat = vnode->stats;

        for (size_t i = 0; i < childCount; i++) {
            if (strncmp(newName, fileNames[i], newNameLength) == 0 &&
                    fileNames[i][newNameLength] == '\0') {
                struct stat childStat = childNodes[i]->stats;
                if (!S_ISDIR(vnodeStat.st_mode) && S_ISDIR(childStat.st_mode)) {
                    errno = EISDIR;
                    return -1;
                }
                if (S_ISDIR(vnodeStat.st_mode) && !S_ISDIR(childStat.st_mode)) {
                    errno = ENOTDIR;
                    return -1;
                }

                if (unlinkUnlocked(newName, AT_REMOVEDIR | AT_REMOVEFILE) < 0) {
                    return -1;
                }

                continue;
            }
        }

        if (linkUnlocked(newName, newNameLength, vnode) < 0) return -1;
        if (S_ISDIR(vnodeStat.st_mode)) {
            ((Reference<DirectoryVnode>) vnode)->parent = this;
        }
    }

    oldDir->unlinkUnlocked(oldName, 0);
    return 0;
}

Reference<Vnode> DirectoryVnode::resolve() {
    AutoLock lock(&mutex);

    if (mounted) return mounted->getRootDir();
    return this;
}

int DirectoryVnode::symlink(const char* linkTarget, const char* name) {
    AutoLock lock(&mutex);

    Reference<Vnode> symlink = new SymlinkVnode(linkTarget, stats.st_dev);
    if (!symlink) return -1;
    return linkUnlocked(name, strcspn(name, "/"), symlink);
}

int DirectoryVnode::unlink(const char* name, int flags) {
    AutoLock lock(&mutex);
    return unlinkUnlocked(name, flags);
}

int DirectoryVnode::unlinkUnlocked(const char* name, int flags) {
    size_t nameLength = strcspn(name, "/");
    for (size_t i = 0; i < childCount; i++) {
        if (strncmp(name, fileNames[i], nameLength) == 0 &&
                fileNames[i][nameLength] == '\0') {
            Reference<Vnode> vnode = childNodes[i];
            AutoLock lock(&vnode->mutex);
            struct stat vnodeStat = vnode->stats;

            // The syscall routine will always set either AT_REMOVEFILE or
            // AT_REMOVEDIR. If no flags are set we remove the entry
            // unconditionally.
            if (flags) {
                if (S_ISDIR(vnodeStat.st_mode) && !(flags & AT_REMOVEDIR)) {
                    errno = EPERM;
                    return -1;
                }
                if (!S_ISDIR(vnodeStat.st_mode) &&
                        (!(flags & AT_REMOVEFILE) || name[nameLength] == '/')) {
                    errno = ENOTDIR;
                    return -1;
                }

                if (!vnode->onUnlink(false)) return -1;
            } else {
                vnode->onUnlink(true);
            }

            if (S_ISDIR(vnode->stats.st_mode)) {
                stats.st_nlink--;
            }

            free(fileNames[i]);
            if (i != childCount - 1) {
                childNodes[i] = childNodes[childCount - 1];
                fileNames[i] = fileNames[childCount - 1];
            }
            childNodes[--childCount].~Reference();

            // Resize the list. Reallocation failure is not an error because we
            // are just making the list smaller.
            Reference<Vnode>* newChildNodes = (Reference<Vnode>*)
                    realloc(childNodes, childCount * sizeof(Reference<Vnode>));
            char** newFileNames = (char**) realloc(fileNames, childCount *
                    sizeof(const char*));
            if (newChildNodes) {
                childNodes = newChildNodes;
            }
            if (newFileNames) {
                fileNames = newFileNames;
            }

            updateTimestamps(false, true, true);
            return 0;
        }
    }

    errno = ENOENT;
    return -1;
}

int DirectoryVnode::unmount() {
    AutoLock lock(&mutex);

    if (!mounted) {
        errno = EINVAL;
        return -1;
    }

    if (!mounted->onUnmount()) return -1;

    delete mounted;
    mounted = nullptr;
    return 0;
}
