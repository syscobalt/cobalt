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

#include <cobalt/kernel/console.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/panic.h>
#include <cobalt/kernel/registers.h>

static void panic(const char* file, unsigned int line, const char* func,
        const char* format, va_list ap) {
    Interrupts::disable();
    console->unlock();
    Log::printf("\n\e[1;37;41mKERNEL PANIC\n");
    Log::vprintf(format, ap);
    Log::printf("\nat %s (%s:%u)\n", func, file, line);
}

NORETURN void panic(const char* file, unsigned int line, const char* func,
        const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    panic(file, line, func, format, ap);
    va_end(ap);

    console->display->onPanic();
    while (true) asm ("hlt");
}

NORETURN void panic(const char* file, unsigned int line, const char* func,
        const InterruptContext* context, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    panic(file, line, func, format, ap);
    va_end(ap);
    Registers::dumpInterruptContext(context);

    console->display->onPanic();
    while (true) asm ("hlt");
}
