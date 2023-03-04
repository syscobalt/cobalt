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

#include <cobalt/kernel/blockcache.h>
#include <cobalt/kernel/interrupts.h>

namespace AtaController {
    void initialize(uint8_t bus, uint8_t device, uint8_t function);
};

class AtaChannel {
public:
    AtaChannel(uint16_t iobase, uint16_t ctrlbase, uint16_t busmasterBase,
            unsigned int irq, paddr_t prdPhys, vaddr_t prdVirt);
    bool flushCache(bool secondary);
    void identifyDevice(bool secondary);
    void onIrq(const InterruptContext* context);
    bool readSectors(char* buffer, size_t sectorCount, uint64_t lba,
            bool secondary, uint64_t sectorSize);
    bool writeSectors(const char* buffer, size_t sectorCount, uint64_t lba,
            bool secondary, uint64_t sectorSize);
private:
    bool finishDmaTransfer();
    bool setSectors(size_t sectorCount, uint64_t lba, bool secondary);
private:
    kthread_mutex_t mutex;
    uint16_t iobase;
    uint16_t ctrlbase;
    uint16_t busmasterBase;
    paddr_t prdPhys;
    vaddr_t prdVirt;
    paddr_t dmaRegion;
    vaddr_t dmaMapped;
    IrqHandler irqHandler;
    bool awaitingInterrupt;
    bool dmaInProgress;
    bool error;
};

class AtaDevice : public BlockCacheDevice {
public:
    AtaDevice(AtaChannel* channel, bool secondary, uint64_t sectors,
            uint64_t sectorSize, bool lba48Supported);
    off_t lseek(off_t offset, int whence) override;
    short poll() override;
    int sync(int flags) override;
protected:
    bool readUncached(void* buffer, size_t size, off_t offset, int flags)
            override;
    bool writeUncached(const void* buffer, size_t size, off_t offset, int flags)
            override;
private:
    AtaChannel* channel;
    uint64_t sectors;
    uint64_t sectorSize;
    bool secondary;
    bool lba48Supported;
};
