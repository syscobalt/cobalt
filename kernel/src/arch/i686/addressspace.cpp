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

#include <assert.h>
#include <string.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/physicalmemory.h>

#define RECURSIVE_MAPPING 0xFFC00000
#define CURRENT_PAGE_DIR_MAPPING (RECURSIVE_MAPPING + 0x3FF000)

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_WRITE_COMBINING (1 << 7)

extern "C" {
extern symbol_t bootstrapBegin;
extern symbol_t bootstrapEnd;
extern symbol_t kernelPageDirectory;
extern symbol_t kernelVirtualBegin;
extern symbol_t kernelReadOnlyEnd;
extern symbol_t kernelVirtualEnd;
}

static kthread_mutex_t listMutex = KTHREAD_MUTEX_INITIALIZER;
static char _kernelMappingArea[PAGESIZE] ALIGNED(PAGESIZE);

// We need to create the initial kernel segments at compile time because
// they are needed before memory allocations are possible.
static MemorySegment segments[] = {
    MemorySegment(0, 0xC0000000, PROT_NONE, nullptr, &segments[1]),
    MemorySegment((vaddr_t) &kernelVirtualBegin, (vaddr_t) &kernelReadOnlyEnd -
            (vaddr_t) &kernelVirtualBegin, PROT_READ | PROT_EXEC, &segments[0],
            &segments[2]),
    MemorySegment((vaddr_t) &kernelReadOnlyEnd, (vaddr_t) &kernelVirtualEnd -
            (vaddr_t) &kernelReadOnlyEnd, PROT_READ | PROT_WRITE, &segments[1],
            &segments[3]),
    MemorySegment(RECURSIVE_MAPPING, -RECURSIVE_MAPPING, PROT_READ | PROT_WRITE,
            &segments[2], nullptr),
};

static inline void addressToIndex(
        vaddr_t virtualAddress, size_t& pdIndex, size_t& ptIndex) {
    assert(PAGE_ALIGNED(virtualAddress));
    pdIndex = virtualAddress >> 22;
    ptIndex = (virtualAddress >> 12) & 0x3FF;
}

static inline int protectionToFlags(int protection) {
    int flags = PAGE_PRESENT;
    if (protection & PROT_WRITE) flags |= PAGE_WRITABLE;
    if (protection & PROT_WRITE_COMBINING && AddressSpace::patSupported) {
        flags |= PAGE_WRITE_COMBINING;
    }
    return flags;
}

AddressSpace::AddressSpace() {
    if (this == kernelSpace) {
        pageDir = (paddr_t) &kernelPageDirectory;
        mappingArea = (vaddr_t) _kernelMappingArea;
        firstSegment = segments;
        prev = nullptr;
        next = nullptr;
    } else {
        pageDir = PhysicalMemory::popPageFrame();
        if (!pageDir) FAIL_CONSTRUCTOR;

        firstSegment = new MemorySegment(0, PAGESIZE, PROT_NONE | SEG_NOUNMAP,
                nullptr, nullptr);
        if (!firstSegment) FAIL_CONSTRUCTOR;
        if (!MemorySegment::addSegment(firstSegment, 0xC0000000, -0xC0000000,
                PROT_NONE | SEG_NOUNMAP)) {
            FAIL_CONSTRUCTOR;
        }
        mappingArea = MemorySegment::findAndAddNewSegment(
                kernelSpace->firstSegment, PAGESIZE, PROT_NONE);
        if (!mappingArea) FAIL_CONSTRUCTOR;

        AutoLock lock(&listMutex);
        next = kernelSpace->next;
        if (next) {
            next->prev = this;
        }
        prev = kernelSpace;
        kernelSpace->next = this;

        // Copy the kernel page directory into the new address space.
        kernelSpace->mapAt(kernelSpace->mappingArea, pageDir, PROT_WRITE);
        memset((void*) kernelSpace->mappingArea, 0, 0xC00);
        memcpy((void*) (kernelSpace->mappingArea + 0xC00),
                (const void*) (CURRENT_PAGE_DIR_MAPPING + 0xC00), 0x3FC);
        *((paddr_t*) (kernelSpace->mappingArea + 0xFFC)) = pageDir |
                PAGE_PRESENT | PAGE_WRITABLE;
        kernelSpace->unmap(kernelSpace->mappingArea);
    }

    mutex = KTHREAD_MUTEX_INITIALIZER;
}

AddressSpace::~AddressSpace() {
    if (!pageDir) return;
    if (!__constructionFailed) {
        kthread_mutex_lock(&listMutex);
        prev->next = next;
        if (next) {
            next->prev = prev;
        }
        kthread_mutex_unlock(&listMutex);
    }

    MemorySegment* currentSegment = firstSegment;

    while (currentSegment) {
        MemorySegment* next = currentSegment->next;

        if (!(currentSegment->flags & SEG_NOUNMAP)) {
            unmapMemory(currentSegment->address, currentSegment->size);
        }
        currentSegment = next;
    }

    if (!__constructionFailed) {
        // Free the page tables.
        uintptr_t* mapped = (uintptr_t*) kernelSpace->mapAt(mappingArea,
                pageDir, PROT_READ);
        for (size_t i = 0; i < 768; i++) {
            paddr_t pt = mapped[i] & ~PAGE_MISALIGN;
            if (pt) {
                PhysicalMemory::pushPageFrame(pt);
            }
        }
        kernelSpace->unmap(mappingArea);
        MemorySegment::removeSegment(kernelSpace->firstSegment, mappingArea,
                PAGESIZE);
    }
    if (firstSegment) {
        if (firstSegment->next) {
            MemorySegment::deallocateSegment(firstSegment->next);
        }
        delete firstSegment;
    }
    PhysicalMemory::pushPageFrame(pageDir);
}

void AddressSpace::initialize() {
    // Unmap the bootstrap sections
    vaddr_t p = (vaddr_t) &bootstrapBegin;

    while (p < (vaddr_t) &bootstrapEnd) {
        kernelSpace->unmap(p);
        if (p < (paddr_t) &kernelPageDirectory) {
            PhysicalMemory::pushPageFrame(p);
        }
        p += PAGESIZE;
    }

    // Remove the mapping for the bootstrap page table
    // This is the first page table, so know it is mapped at RECURSIVE_MAPPING.
    kernelSpace->unmap(RECURSIVE_MAPPING);

    uint32_t eax = 1;
    uint32_t edx;
    asm("cpuid" : "+a"(eax), "=d"(edx) :: "ebx", "ecx");
    patSupported = edx & (1 << 16);

    if (patSupported) {
        uint32_t patLow;
        uint32_t patHigh;
        asm("rdmsr" : "=a"(patLow), "=d"(patHigh) : "c"(0x277));
        // Set PAT entry 4 to write combining.
        patHigh = (patHigh & 0xFFFFFF00) | 0x01;
        asm("wrmsr" :: "a"(patLow), "d"(patHigh), "c"(0x277));
    }
}

void AddressSpace::activate() {
    activeAddressSpace = this;
    asm ("mov %0, %%cr3" :: "r"(pageDir));
}

paddr_t AddressSpace::getPhysicalAddress(vaddr_t virtualAddress) {
    if (this == kernelSpace && virtualAddress < 0xC0000000) return 0;

    size_t pdIndex;
    size_t ptIndex;
    addressToIndex(virtualAddress, pdIndex, ptIndex);

    if (isActive()) {
        uintptr_t* pageDirectory = (uintptr_t*) CURRENT_PAGE_DIR_MAPPING;
        if (!pageDirectory[pdIndex]) return 0;
        uintptr_t* pageTable =
                (uintptr_t*) (RECURSIVE_MAPPING + PAGESIZE * pdIndex);
        return pageTable[ptIndex] & ~PAGE_MISALIGN;
    } else {
        uintptr_t* pageDirectory = (uintptr_t*) kernelSpace->mapAt(mappingArea,
                pageDir, PROT_READ);
        uintptr_t pdEntry = pageDirectory[pdIndex];
        kernelSpace->unmap(mappingArea);
        if (!pdEntry) return 0;

        uintptr_t* pageTable = (uintptr_t*) kernelSpace->mapAt(mappingArea,
                pdEntry & ~PAGE_MISALIGN, PROT_READ);
        paddr_t result = pageTable[ptIndex] & ~PAGE_MISALIGN;
        kernelSpace->unmap(mappingArea);
        return result;
    }
}

vaddr_t AddressSpace::mapAt(
        vaddr_t virtualAddress, paddr_t physicalAddress, int protection) {
    assert(PAGE_ALIGNED(physicalAddress));

    int flags = protectionToFlags(protection);

    if (this != kernelSpace) {
        // Memory in user space is always accessible by user.
        flags |= PAGE_USER;
    }
    if (!physicalAddress) {
        flags = 0;
    }

    size_t pdIndex;
    size_t ptIndex;
    addressToIndex(virtualAddress, pdIndex, ptIndex);

    uintptr_t* pageDirectory;
    uintptr_t* pageTable = nullptr;

    if (isActive()) {
        pageDirectory = (uintptr_t*) CURRENT_PAGE_DIR_MAPPING;
        pageTable = (uintptr_t*) (RECURSIVE_MAPPING + PAGESIZE * pdIndex);
    } else {
        pageDirectory = (uintptr_t*) kernelSpace->mapAt(mappingArea, pageDir,
                PROT_READ | PROT_WRITE);
    }

    if (!pageDirectory[pdIndex]) {
        // Allocate a new page table and map it in the page directory
        paddr_t pageTablePhys = PhysicalMemory::popPageFrame();
        if (!pageTablePhys) {
            if (!isActive()) {
                kernelSpace->unmap(mappingArea);
            }
            return 0;
        }

        int pdFlags = PAGE_PRESENT | PAGE_WRITABLE;
        if (this != kernelSpace) pdFlags |= PAGE_USER;

        if (this == kernelSpace) {
            // We need to map that page table in all address spaces
            AutoLock lock(&listMutex);

            AddressSpace* addressSpace = kernelSpace;
            while (addressSpace) {
                uintptr_t* pd = (uintptr_t*)
                        kernelSpace->mapAt(kernelSpace->mappingArea,
                        addressSpace->pageDir, PROT_WRITE);
                pd[pdIndex] = pageTablePhys | pdFlags;
                kernelSpace->unmap(kernelSpace->mappingArea);
                addressSpace = addressSpace->next;
            }
        } else if (isActive()) {
            pageDirectory[pdIndex] = pageTablePhys | pdFlags;
            asm ("invlpg (%0)" :: "r"(pageTable));
        } else {
            pageDirectory[pdIndex] = pageTablePhys | pdFlags;
            kernelSpace->unmap(mappingArea);
            pageTable = (uintptr_t*) kernelSpace->mapAt(mappingArea,
                    pageTablePhys, PROT_READ | PROT_WRITE);
        }

        memset(pageTable, 0, PAGESIZE);

    } else if (!isActive()) {
        paddr_t pageTablePhys = pageDirectory[pdIndex] & ~PAGE_MISALIGN;
        kernelSpace->unmap(mappingArea);
        pageTable = (uintptr_t*) kernelSpace->mapAt(mappingArea, pageTablePhys,
                PROT_READ | PROT_WRITE);
    }

    pageTable[ptIndex] = physicalAddress | flags;

    if (isActive()) {
        asm ("invlpg (%0)" :: "r"(virtualAddress));
    } else {
        kernelSpace->unmap(mappingArea);
    }

    return virtualAddress;
}
