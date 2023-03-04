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

#ifndef KERNEL_AHCI_H
#define KERNEL_AHCI_H

#include <cobalt/kernel/blockcache.h>
#include <cobalt/kernel/interrupts.h>

namespace Ahci {
void initialize(uint8_t bus, uint8_t device, uint8_t function);
}

class AhciDevice : public BlockCacheDevice {
public:
    AhciDevice(vaddr_t portRegisters, paddr_t portMemPhys, vaddr_t portMemVirt);
    bool identify();
    off_t lseek(off_t offset, int whence) override;
    void onIrq(const InterruptContext* context, uint32_t interruptStatus);
    short poll() override;
    int sync(int flags) override;
protected:
    bool readUncached(void* buffer, size_t size, off_t offset, int flags)
            override;
    bool writeUncached(const void* buffer, size_t size, off_t offset, int flags)
            override;
private:
    bool finishDmaTransfer();
    uint32_t readRegister(size_t offset);
    bool sendDmaCommand(uint8_t command, paddr_t physicalAddress, size_t size,
            bool write, uint64_t lba, uint16_t blockCount);
    void writeRegister(size_t offset, uint32_t value);
private:
    vaddr_t portRegisters;
    paddr_t portMemPhys;
    vaddr_t portMemVirt;
    uint32_t error;
    uint64_t sectors;
    uint64_t sectorSize;
    bool awaitingInterrupt;
    bool dmaInProgress;
};

#endif
