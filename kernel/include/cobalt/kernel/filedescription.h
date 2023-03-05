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

#ifndef KERNEL_FILEDESCRIPTION_H
#define KERNEL_FILEDESCRIPTION_H

#include <cobalt/kernel/vnode.h>

class FileDescription : public ReferenceCounted {
public:
    FileDescription(const Reference<Vnode>& vnode, int flags);
    ~FileDescription();
    Reference<FileDescription> accept4(struct sockaddr* address,
            socklen_t* length, int flags);
    int bind(const struct sockaddr* address, socklen_t length);
    int connect(const struct sockaddr* address, socklen_t length);
    int fcntl(int cmd, int param);
    ssize_t getdents(void* buffer, size_t size, int flags);
    off_t lseek(off_t offset, int whence);
    Reference<FileDescription> openat(const char* path, int flags,
            mode_t mode);
    ssize_t read(void* buffer, size_t size);
    int tcgetattr(struct termios* result);
    int tcsetattr(int flags, const struct termios* termio);
    ssize_t write(const void* buffer, size_t size);
public:
    Reference<Vnode> vnode;
private:
    kthread_mutex_t mutex;
    void* dents;
    size_t dentsSize;
    off_t offset;
    int fileFlags;
};

#endif
