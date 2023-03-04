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
#include <string.h>
#include <cobalt/kernel/clock.h>
#include <cobalt/kernel/kernel.h>

static bool initialized;
static bool rdrandSupported;

static void initialize(void) {
#if defined(__i386__) || defined(__x86_64__)
    uint32_t eax = 1;
    uint32_t ecx = 0;
    asm("cpuid" : "+a"(eax), "+c"(ecx) :: "ebx", "edx");
    rdrandSupported = ecx & (1 << 30);
#endif
    initialized = true;
}

extern "C" int getentropy(void* buffer, size_t size) {
    assert(size <= GETENTROPY_MAX);
    if (!initialized) {
        initialize();
    }

    size_t i = 0;
    while (i < size) {
        const void* entropy;
        struct timespec ts[2];
        size_t amount = size - i;
        bool success = false;

#if defined(__i386__) || defined(__x86_64__)
        long value;
        // Use the rdrand instruction if it is available.
        if (rdrandSupported) {
            for (size_t j = 0; j < 10; j++) {
                asm("rdrand %0" : "=r"(value), "=@ccc"(success));
                if (success) break;
            }
            if (amount > sizeof(long)) amount = sizeof(long);
            entropy = &value;
        }
#endif

        if (!success) {
            // If rdrand is not available fall back to using the current time.
            // This is not very secure though.
            // TODO: Gather our own entropy so we don't need to rely on rdrand.
            Clock::get(CLOCK_REALTIME)->getTime(&ts[0]);
            Clock::get(CLOCK_MONOTONIC)->getTime(&ts[1]);
            if (amount > sizeof(ts)) amount = sizeof(ts);
            entropy = ts;
        }
        memcpy((char*) buffer + i, entropy, amount);
        i += amount;
    }
    return 0;
}
