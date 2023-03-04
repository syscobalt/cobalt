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

#include <assert.h>
#include <stdlib.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/kthread.h>
#include <cobalt/kernel/panic.h>

static kthread_mutex_t heapLock = KTHREAD_MUTEX_INITIALIZER;
static kthread_mutex_t randomLock = KTHREAD_MUTEX_INITIALIZER;

extern "C" NORETURN void abort(void) {
    PANIC("Abort was called");
}

extern "C" NORETURN void __assertionFailure(const char* assertion,
        const char* file, unsigned int line, const char* func) {
    panic(file, line, func, "Assertion failed: '%s'", assertion);
}

extern "C" NORETURN void __handleUbsan(const char* file, uint32_t line,
        uint32_t column, const char* message) {
    PANIC("Undefined behavior detected: %s\nat %s:%u:%u", message, file, line,
            column);
}

extern "C" void __lockHeap(void) {
    kthread_mutex_lock(&heapLock);
}

extern "C" void __lockRandom(void) {
    kthread_mutex_lock(&randomLock);
}

extern "C" void* __mapMemory(size_t size) {
    return (void*) kernelSpace->mapMemory(size, PROT_READ | PROT_WRITE);
}

extern "C" NORETURN void __stack_chk_fail(void) {
    PANIC("Stack smashing detected");
}

extern "C" void __unlockHeap(void) {
    kthread_mutex_unlock(&heapLock);
}

extern "C" void __unlockRandom(void) {
    kthread_mutex_unlock(&randomLock);
}

extern "C" void __unmapMemory(void* addr, size_t size) {
    kernelSpace->unmapMemory((vaddr_t) addr, size);
}
