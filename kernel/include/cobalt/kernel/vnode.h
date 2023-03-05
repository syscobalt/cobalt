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

#ifndef KERNEL_VNODE_H
#define KERNEL_VNODE_H

#include <sys/socket.h>
#include <sys/types.h>
#include <cobalt/stat.h>
#include <cobalt/kernel/kthread.h>
#include <cobalt/kernel/refcount.h>

class FileSystem;

class Vnode : public ReferenceCounted {
public:
    virtual Reference<Vnode> accept(struct sockaddr* address,
            socklen_t* length, int fileFlags);
    virtual int bind(const struct sockaddr* address, socklen_t length,
            int flags);
    virtual int chmod(mode_t mode);
    virtual int chown(uid_t uid, gid_t gid);
    virtual int connect(const struct sockaddr* address, socklen_t length,
            int flags);
    virtual int devctl(int command, void* restrict data, size_t size,
            int* restrict info);
    virtual int ftruncate(off_t length);
    virtual Reference<Vnode> getChildNode(const char* path);
    virtual Reference<Vnode> getChildNode(const char* path, size_t length);
    virtual size_t getDirectoryEntries(void** buffer, int flags);
    virtual char* getLinkTarget();
    virtual int isatty();
    virtual bool isSeekable();
    virtual int link(const char* name, const Reference<Vnode>& vnode);
    virtual int listen(int backlog);
    virtual off_t lseek(off_t offset, int whence);
    virtual int mkdir(const char* name, mode_t mode);
    virtual int mount(FileSystem* filesystem);
    virtual void onLink();
    virtual bool onUnlink(bool force);
    virtual Reference<Vnode> open(const char* name, int flags, mode_t mode);
    virtual long pathconf(int name);
    virtual short poll();
    virtual ssize_t pread(void* buffer, size_t size, off_t offset, int flags);
    virtual ssize_t pwrite(const void* buffer, size_t size, off_t offset,
                int flags);
    virtual ssize_t read(void* buffer, size_t size, int flags);
    virtual ssize_t readlink(char* buffer, size_t size);
    virtual int rename(const Reference<Vnode>& oldDirectory,
            const char* oldName, const char* newName);
    virtual Reference<Vnode> resolve();
    virtual int stat(struct stat* result);
    struct stat stat();
    virtual int symlink(const char* linkTarget, const char* name);
    virtual int sync(int flags);
    virtual int tcgetattr(struct termios* result);
    virtual int tcsetattr(int flags, const struct termios* termio);
    virtual int unlink(const char* name, int flags);
    virtual int unmount();
    void updateTimestampsLocked(bool access, bool status, bool modification);
    virtual int utimens(struct timespec atime, struct timespec mtime);
    virtual ssize_t write(const void* buffer, size_t size, int flags);
    virtual ~Vnode();
protected:
    Vnode(mode_t mode, dev_t dev);
    virtual void updateTimestamps(bool access, bool status, bool modification);
public:
    kthread_mutex_t mutex;
    struct stat stats;
};

Reference<Vnode> resolvePath(const Reference<Vnode>& vnode, const char* path,
        bool followFinalSymlink = true);
Reference<Vnode> resolvePathExceptLastComponent(const Reference<Vnode>& vnode,
        const char* path, const char** lastComponent,
        bool followFinalSymlink = false);

#endif
