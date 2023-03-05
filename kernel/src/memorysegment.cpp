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

#include <assert.h>
#include <string.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/kthread.h>
#include <cobalt/kernel/memorysegment.h>
#include <cobalt/kernel/physicalmemory.h>

static char segmentsPage[PAGESIZE] ALIGNED(PAGESIZE) = {0};
static kthread_mutex_t mutex = KTHREAD_MUTEX_INITIALIZER;

static inline size_t getFreeSpaceAfter(MemorySegment* segment) {
    vaddr_t nextAddress = segment->next ? segment->next->address : 0;
    return nextAddress - (segment->address + segment->size);
}

MemorySegment::MemorySegment(vaddr_t address, size_t size, int flags,
        MemorySegment* prev, MemorySegment* next) {
    this->address = address;
    this->size = size;
    this->flags = flags;
    this->prev = prev;
    this->next = next;
}

void MemorySegment::addSegment(MemorySegment* firstSegment,
        MemorySegment* newSegment) {
    vaddr_t endAddress = newSegment->address + newSegment->size;

    MemorySegment* currentSegment = firstSegment;

    while (currentSegment->next &&
            currentSegment->next->address < endAddress) {
        currentSegment = currentSegment->next;
    }

    assert(currentSegment->address + currentSegment->size <=
            newSegment->address);
    assert(!currentSegment->next ||
            currentSegment->next->address >= endAddress);

    newSegment->prev = currentSegment;
    newSegment->next = currentSegment->next;

    currentSegment->next = newSegment;
    if (newSegment->next) {
        newSegment->next->prev = newSegment;
    }
}

bool MemorySegment::addSegment(MemorySegment* firstSegment, vaddr_t address,
        size_t size, int protection) {
    AutoLock lock(&mutex);
    if (!verifySegmentList()) return false;
    MemorySegment* newSegment = allocateSegment(address, size, protection);
    addSegment(firstSegment, newSegment);
    return true;
}

MemorySegment* MemorySegment::allocateSegment(vaddr_t address, size_t size,
        int flags) {
    assert(PAGE_ALIGNED(address));
    assert(PAGE_ALIGNED(size));
    MemorySegment* current = (MemorySegment*) segmentsPage;

    while (current->address != 0 && current->size != 0) {
        current++;
        if (((uintptr_t) current & PAGE_MISALIGN) ==
                (PAGESIZE - PAGESIZE % sizeof(MemorySegment))) {
            MemorySegment** nextPage = (MemorySegment**) current;
            assert(*nextPage != nullptr);
            current = *nextPage;
        }
    }

    current->address = address;
    current->size = size;
    current->flags = flags;

    return current;
}

void MemorySegment::deallocateSegment(MemorySegment* segment) {
    memset(segment, 0, sizeof(MemorySegment));
}

void MemorySegment::removeSegment(MemorySegment* firstSegment, vaddr_t address,
        size_t size) {
    AutoLock lock(&mutex);
    MemorySegment* currentSegment = firstSegment;
    vaddr_t endAddress = address + size;

    while (currentSegment &&
            currentSegment->address + currentSegment->size <= address &&
            currentSegment->address + currentSegment->size != 0) {
        currentSegment = currentSegment->next;
    }

    while (size && currentSegment) {
        if (currentSegment->address > address) {
            if (currentSegment->address > endAddress && endAddress != 0) {
                return;
            }
            size -= currentSegment->address - address;
            address = currentSegment->address;
        }

        if (currentSegment->address == address &&
                currentSegment->size <= size) {
            // Delete the whole segment
            address += currentSegment->size;
            size -= currentSegment->size;

            MemorySegment* next = currentSegment->next;
            if (next) {
                next->prev = currentSegment->prev;
            }
            if (currentSegment->prev) {
                currentSegment->prev->next = next;
            }

            deallocateSegment(currentSegment);
            currentSegment = next;
            continue;
        } else if (currentSegment->address == address &&
                currentSegment->size > size) {
            currentSegment->address += size;
            currentSegment->size -= size;
            size = 0;
        } else if (size + (address - currentSegment->address) >=
                currentSegment->size) {
            size_t diff = currentSegment->address + currentSegment->size -
                    address;
            currentSegment->size -= diff;
            size -= diff;
            address += diff;
        } else {
            if (!verifySegmentList()) {
                // We are so low on memory that we cannot keep track of segments
                // and therefore have to leak virtual memory.
                return;
            }

            // Split the segment
            size_t firstSize = address - currentSegment->address;
            size_t secondSize = currentSegment->size - firstSize - size;

            MemorySegment* newSegment = allocateSegment(endAddress, secondSize,
                    currentSegment->flags);

            newSegment->prev = currentSegment;
            newSegment->next = currentSegment->next;
            if (newSegment->next) {
                newSegment->next->prev = newSegment;
            }

            currentSegment->next = newSegment;
            currentSegment->size = firstSize;
        }

        currentSegment = currentSegment->next;
    }
}

MemorySegment* MemorySegment::findFreeSegment(MemorySegment* firstSegment,
        size_t size) {
    MemorySegment* currentSegment = firstSegment;

    while (currentSegment && getFreeSpaceAfter(currentSegment) < size) {
        currentSegment = currentSegment->next;
    }

    return currentSegment;
}

vaddr_t MemorySegment::findAndAddNewSegment(MemorySegment* firstSegment,
        size_t size, int protection) {
    AutoLock lock(&mutex);

    if (!verifySegmentList()) return 0;
    MemorySegment* segment = findFreeSegment(firstSegment, size);
    if (!segment) return 0;

    vaddr_t address = segment->address + segment->size;
    if (segment->flags == protection) {
        segment->size += size;
        return address;
    }

    MemorySegment* newSegment = allocateSegment(address, size, protection);
    addSegment(firstSegment, newSegment);
    return address;
}

bool MemorySegment::verifySegmentList() {
    MemorySegment* current = (MemorySegment*) segmentsPage;
    MemorySegment** nextPage;

    int freeSegmentSpaceFound = 0;
    MemorySegment* freeSegment = nullptr;

    while (true) {
        if (current->address == 0 && current->size == 0) {
            freeSegment = current;
            freeSegmentSpaceFound++;
        }

        current++;
        if (((uintptr_t) current & PAGE_MISALIGN) ==
                (PAGESIZE - PAGESIZE % sizeof(MemorySegment))) {
            nextPage = (MemorySegment**) current;
            if (!*nextPage) break;
            current = *nextPage;
        }
    }

    assert(freeSegmentSpaceFound > 0);

    if (freeSegmentSpaceFound == 1) {
        current = findFreeSegment(kernelSpace->firstSegment, PAGESIZE);
        if (!current) return false;
        vaddr_t address = current->address + current->size;
        paddr_t physical = PhysicalMemory::popPageFrame();
        if (!physical) return false;
        if (!kernelSpace->mapAt(address, physical, PROT_READ | PROT_WRITE)) {
            PhysicalMemory::pushPageFrame(physical);
            return false;
        }
        *nextPage = (MemorySegment*) address;

        memset(*nextPage, 0, PAGESIZE);

        if (current->flags == (PROT_READ | PROT_WRITE)) {
            current->size += PAGESIZE;
        } else {
            freeSegment->address = address;
            freeSegment->size = PAGESIZE;
            freeSegment->flags = PROT_READ | PROT_WRITE;
            addSegment(kernelSpace->firstSegment, freeSegment);
        }
    }

    return true;
}
