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

#ifndef KERNEL_PARTITION_H
#define KERNEL_PARTITION_H

#include <cobalt/kernel/vnode.h>

class Partition : public Vnode {
public:
    Partition(const Reference<Vnode>& device, off_t offset, size_t size);
    bool isSeekable() override;
    off_t lseek(off_t offset, int whence) override;
    short poll() override;
    ssize_t pread(void* buffer, size_t size, off_t offset, int flags) override;
    ssize_t pwrite(const void* buffer, size_t size, off_t offset, int flags)
            override;
    int sync(int flags) override;
public:
    static void scanPartitions(const Reference<Vnode>& device,
            const char* deviceName, size_t sectorSize);
private:
    Reference<Vnode> device;
    off_t partitionOffset;
};

#endif
