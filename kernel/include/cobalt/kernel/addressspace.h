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

#ifndef KERNEL_ADDRESSSPACE_H
#define KERNEL_ADDRESSSPACE_H

#include <cobalt/mman.h>
#include <cobalt/kernel/kthread.h>
#include <cobalt/kernel/memorysegment.h>

#define PROT_WRITE_COMBINING (1 << 17)

class AddressSpace : public ConstructorMayFail {
public:
    AddressSpace();
    ~AddressSpace();
    void activate();
    AddressSpace* fork();
    paddr_t getPhysicalAddress(vaddr_t virtualAddress);
    vaddr_t mapAt(vaddr_t virtualAddress, paddr_t physicalAddress,
            int protection);
    vaddr_t mapFromOtherAddressSpace(AddressSpace* sourceSpace,
            vaddr_t sourceVirtualAddress, size_t size, int protection);
    vaddr_t mapMemory(size_t size, int protection);
    vaddr_t mapMemory(vaddr_t virtualAddress, size_t size, int protection);
    vaddr_t mapPhysical(paddr_t physicalAddress, size_t size, int protection);
    vaddr_t mapUnaligned(paddr_t physicalAddress, size_t size, int protection,
            vaddr_t& mapping, size_t& mapSize);
    void unmapMemory(vaddr_t virtualAddress, size_t size);
    void unmapPhysical(vaddr_t firstVirtualAddress, size_t size);
private:
    bool isActive();
    vaddr_t mapMemoryInternal(vaddr_t virtualAddress, size_t size,
            int protection);
    void unmap(vaddr_t virtualAddress);
public:
    MemorySegment* firstSegment;
private:
    AddressSpace* prev;
    AddressSpace* next;
    kthread_mutex_t mutex;
    vaddr_t mappingArea;
#ifdef __i386__
    paddr_t pageDir;
#elif defined(__x86_64__)
    paddr_t pml4;
#endif
public:
    static void initialize();
    static bool patSupported;
private:
    static AddressSpace* activeAddressSpace;
};

// Global variable for the kernel's address space
extern AddressSpace* const kernelSpace;

#endif
