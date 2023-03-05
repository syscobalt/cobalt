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

#include <cobalt/kernel/log.h>
#include <cobalt/kernel/registers.h>

void Registers::dumpInterruptContext(const InterruptContext* context) {
    Log::printf("EAX: 0x%.8lX, EBX: 0x%.8lX, ECX: 0x%.8lX, EDX: 0x%.8lX\n",
            context->eax, context->ebx, context->ecx, context->edx);
    Log::printf("ESI: 0x%.8lX, EDI: 0x%.8lX, EBP: 0x%.8lX\n",
            context->esi, context->edi, context->ebp);
    Log::printf("ERROR: 0x%.4lX, EFLAGS: 0x%.8lX\n", context->error,
            context->eflags);
    Log::printf("CS:  0x%.4lX, EIP: 0x%.8lX\n", context->cs, context->eip);
    if (context->cs != 0x8) {
        Log::printf("SS: 0x%.4lX, ESP: 0x%.8lX\n", context->ss, context->esp);
    }
}

void Registers::restore(InterruptContext* context,
        const __registers_t* registers) {
    context->eax = registers->__eax;
    context->ebx = registers->__ebx;
    context->ecx = registers->__ecx;
    context->edx = registers->__edx;
    context->esi = registers->__esi;
    context->edi = registers->__edi;
    context->ebp = registers->__ebp;
    context->eip = registers->__eip;
    context->eflags = (registers->__eflags & 0xCD5) | 0x200;
    context->esp = registers->__esp;
    context->cs = 0x1B;
    context->ss = 0x23;
}

void Registers::restoreFpu(const __fpu_t* fpu) {
    asm("fxrstor (%0)" :: "r"(*fpu));
}

void Registers::save(const InterruptContext* context,
        __registers_t* registers) {
    registers->__eax = context->eax;
    registers->__ebx = context->ebx;
    registers->__ecx = context->ecx;
    registers->__edx = context->edx;
    registers->__esi = context->esi;
    registers->__edi = context->edi;
    registers->__ebp = context->ebp;
    registers->__eip = context->eip;
    registers->__eflags = context->eflags;
    registers->__esp = context->esp;
}

void Registers::saveFpu(__fpu_t* fpu) {
    asm("fxsave (%0)" :: "r"(*fpu));
}
