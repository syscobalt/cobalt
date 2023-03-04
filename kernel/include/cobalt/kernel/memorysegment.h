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

#ifndef KERNEL_MEMORYSEGMENT_H
#define KERNEL_MEMORYSEGMENT_H

#include <cobalt/kernel/kernel.h>

#define SEG_NOUNMAP (1 << 16)

class MemorySegment {
public:
    MemorySegment(vaddr_t address, size_t size, int flags, MemorySegment* prev,
            MemorySegment* next);
public:
    vaddr_t address;
    size_t size;
    int flags;
    MemorySegment* prev;
    MemorySegment* next;
public:
    static bool addSegment(MemorySegment* firstSegment, vaddr_t address,
            size_t size, int protection);
    static void deallocateSegment(MemorySegment* segment);
    static void removeSegment(MemorySegment* firstSegment, vaddr_t address,
            size_t size);
    static vaddr_t findAndAddNewSegment(MemorySegment* firstSegment,
            size_t size, int protection);
private:
    static void addSegment(MemorySegment* firstSegment,
            MemorySegment* newSegment);
    static MemorySegment* allocateSegment(vaddr_t address, size_t size,
            int flags);
    static MemorySegment* findFreeSegment(MemorySegment* firstSegment,
            size_t size);
    static bool verifySegmentList();
};

#endif
