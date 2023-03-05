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

#ifndef KERNEL_PCI_H
#define KERNEL_PCI_H

#include <stdint.h>
#include <cobalt/kernel/kernel.h>

struct PciHeader {
    uint16_t vendorId;
    uint16_t deviceId;
    uint16_t command;
    uint16_t status;
    uint8_t revisionId;
    uint8_t progIf;
    uint8_t subclass;
    uint8_t classCode;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t bist;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t cardbusCisPointer;
    uint16_t subsystemVendorId;
    uint16_t subsystemId;
    uint32_t expansionRomBaseAddress;
    uint8_t capabilitiesPointer;
    uint8_t reserved1;
    uint16_t reserved2;
    uint32_t reserved3;
    uint8_t interruptLine;
    uint8_t interruptPin;
    uint8_t minGrant;
    uint8_t maxLatency;
};

namespace Pci {
int getIrq(unsigned int bus, unsigned int device, unsigned int function);
uint32_t readConfig(unsigned int bus, unsigned int device,
        unsigned int function, unsigned int offset);
void writeConfig(unsigned int bus, unsigned int device, unsigned int function,
        unsigned int offset, uint32_t value);
void scanForDevices();
}

#endif
