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

#ifndef KERNEL_BLOCKCACHE_H
#define KERNEL_BLOCKCACHE_H

#include <cobalt/kernel/cache.h>
#include <cobalt/kernel/hashtable.h>
#include <cobalt/kernel/vnode.h>
#include <cobalt/kernel/worker.h>

class BlockCacheDevice : public Vnode, public CacheController {
protected:
    BlockCacheDevice(mode_t mode, dev_t dev);
public:
    void freeUnusedBlocks();
    bool isSeekable() override;
    ssize_t pread(void* buffer, size_t size, off_t offset, int flags) override;
    ssize_t pwrite(const void* buffer, size_t size, off_t offset, int flags)
            override;
    paddr_t reclaimCache() override;
protected:
    virtual bool readUncached(void* buffer, size_t size, off_t offset,
            int flags) = 0;
    virtual bool writeUncached(const void* buffer, size_t size, off_t offset,
            int flags) = 0;
private:
    struct Block {
        Block(vaddr_t address, uint64_t blockNumber);

        vaddr_t address;
        uint64_t blockNumber;
        Block* nextInHashTable;
        Block* prevAccessed;
        Block* nextAccessed;
        Block* nextFree;

        uint64_t hashKey() { return blockNumber; }
    };
    HashTable<Block, uint64_t> blocks;
    Block* blockBuffer[10000];
    kthread_mutex_t cacheMutex;
    Block* freeList;
    Block* leastRecentlyUsed;
    Block* mostRecentlyUsed;
    WorkerJob workerJob;
private:
    void useBlock(Block* block);
};

#endif
