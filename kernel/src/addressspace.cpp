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

#include <string.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/physicalmemory.h>

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)

static AddressSpace _kernelSpace;
AddressSpace* const kernelSpace = &_kernelSpace;
AddressSpace* AddressSpace::activeAddressSpace;
bool AddressSpace::patSupported;

bool AddressSpace::isActive() {
    return this == kernelSpace || this == activeAddressSpace;
}

static kthread_mutex_t forkMutex = KTHREAD_MUTEX_INITIALIZER;

AddressSpace* AddressSpace::fork() {
    AutoLock lock(&forkMutex);

    AddressSpace* result = new AddressSpace();
    if (!result) return nullptr;
    MemorySegment* segment = firstSegment->next;
    while (segment) {
        if (!(segment->flags & SEG_NOUNMAP)) {
            // Copy the segment
            size_t size = segment->size;
            if (!result->mapMemory(segment->address, size, segment->flags)) {
                delete result;
                return nullptr;
            }

            vaddr_t source = kernelSpace->mapFromOtherAddressSpace(this,
                    segment->address, size, PROT_READ);
            if (!source) {
                delete result;
                return nullptr;
            }

            vaddr_t dest = kernelSpace->mapFromOtherAddressSpace(result,
                    segment->address, size, PROT_WRITE);
            if (!dest) {
                kernelSpace->unmapPhysical(source, size);
                delete result;
                return nullptr;
            }

            memcpy((void*) dest, (const void*) source, size);
            kernelSpace->unmapPhysical(source, size);
            kernelSpace->unmapPhysical(dest, size);
        }
        segment = segment->next;
    }

    return result;
}

vaddr_t AddressSpace::mapFromOtherAddressSpace(AddressSpace* sourceSpace,
        vaddr_t sourceVirtualAddress, size_t size, int protection) {
    kthread_mutex_lock(&mutex);
    vaddr_t destination = MemorySegment::findAndAddNewSegment(firstSegment,
            size, protection);
    kthread_mutex_unlock(&mutex);
    if (!destination) return 0;

    for (size_t i = 0 ; i < size; i += PAGESIZE) {
        kthread_mutex_lock(&sourceSpace->mutex);
        paddr_t physicalAddress =
                sourceSpace->getPhysicalAddress(sourceVirtualAddress + i);
        kthread_mutex_unlock(&sourceSpace->mutex);
        kthread_mutex_lock(&mutex);
        if (!mapAt(destination + i, physicalAddress, protection)) {
            for (size_t j = 0; j < i; j += PAGESIZE) {
                unmap(destination + j);
            }
            MemorySegment::removeSegment(firstSegment, destination, size);
            return 0;
        }
        kthread_mutex_unlock(&mutex);
    }


    return destination;
}

vaddr_t AddressSpace::mapMemoryInternal(vaddr_t virtualAddress, size_t size,
        int protection) {
    size_t pages = size / PAGESIZE;

    if (!PhysicalMemory::reserveFrames(pages)) {
        MemorySegment::removeSegment(firstSegment, virtualAddress, size);
        return 0;
    }

    for (size_t i = 0; i < pages; i++) {
        paddr_t physicalAddress = PhysicalMemory::popReserved();
        if (unlikely(!mapAt(virtualAddress + i * PAGESIZE, physicalAddress,
                protection))) {
            PhysicalMemory::unreserveFrames(pages - i - 1);
            kthread_mutex_unlock(&mutex);
            PhysicalMemory::pushPageFrame(physicalAddress);
            kthread_mutex_lock(&mutex);

            for (size_t j = 0; j < i; j++) {
                physicalAddress = getPhysicalAddress(virtualAddress +
                        j * PAGESIZE);
                unmap(virtualAddress + j * PAGESIZE);

                kthread_mutex_unlock(&mutex);
                PhysicalMemory::pushPageFrame(physicalAddress);
                kthread_mutex_lock(&mutex);
            }
            MemorySegment::removeSegment(firstSegment, virtualAddress, size);
            return 0;
        }
    }

    return virtualAddress;
}

vaddr_t AddressSpace::mapMemory(size_t size, int protection) {
    AutoLock lock(&mutex);
    vaddr_t virtualAddress = MemorySegment::findAndAddNewSegment(firstSegment,
            size, protection);
    if (!virtualAddress) return 0;
    return mapMemoryInternal(virtualAddress, size, protection);
}

vaddr_t AddressSpace::mapMemory(vaddr_t virtualAddress, size_t size,
        int protection) {
    AutoLock lock(&mutex);

    if (!MemorySegment::addSegment(firstSegment, virtualAddress, size,
            protection)) {
        return 0;
    }
    return mapMemoryInternal(virtualAddress, size, protection);
}

vaddr_t AddressSpace::mapPhysical(paddr_t physicalAddress, size_t size,
        int protection) {
    AutoLock lock(&mutex);

    vaddr_t virtualAddress = MemorySegment::findAndAddNewSegment(firstSegment,
            size, protection);
    if (!virtualAddress) return 0;
    for (size_t i = 0; i < size; i += PAGESIZE) {
        if (!mapAt(virtualAddress + i, physicalAddress + i, protection)) {
            for (size_t j = 0; j < i; j += PAGESIZE) {
                unmap(virtualAddress + j);
            }
            MemorySegment::removeSegment(firstSegment, virtualAddress, size);
            return 0;
        }
    }

    return virtualAddress;
}

vaddr_t AddressSpace::mapUnaligned(paddr_t physicalAddress, size_t size,
        int protection, vaddr_t& mapping, size_t& mapSize) {
    paddr_t physAligned = physicalAddress & ~PAGE_MISALIGN;
    size_t offset = physicalAddress - physAligned;
    mapSize = ALIGNUP(offset + size, PAGESIZE);

    mapping = mapPhysical(physAligned, mapSize, protection);
    if (!mapping) return 0;
    return mapping + offset;
}

void AddressSpace::unmap(vaddr_t virtualAddress) {
    mapAt(virtualAddress, 0, 0);
}

void AddressSpace::unmapMemory(vaddr_t virtualAddress, size_t size) {
    AutoLock lock(&mutex);

    for (size_t i = 0; i < size; i += PAGESIZE) {
        paddr_t physicalAddress = getPhysicalAddress(virtualAddress + i);
        unmap(virtualAddress + i);

        // Unlock the mutex because PhysicalMemory::pushPageFrame may need to
        // map pages.
        kthread_mutex_unlock(&mutex);
        PhysicalMemory::pushPageFrame(physicalAddress);
        kthread_mutex_lock(&mutex);
    }

    MemorySegment::removeSegment(firstSegment, virtualAddress, size);
}

void AddressSpace::unmapPhysical(vaddr_t virtualAddress, size_t size) {
    AutoLock lock(&mutex);

    for (size_t i = 0; i < size; i += PAGESIZE) {
        unmap(virtualAddress + i);
    }

    MemorySegment::removeSegment(firstSegment, virtualAddress, size);
}
