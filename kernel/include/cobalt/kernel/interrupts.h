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

#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include <cobalt/registers.h>
#include <cobalt/kernel/kernel.h>

struct InterruptContext {
#ifdef __i386__
    __reg_t eax;
    __reg_t ebx;
    __reg_t ecx;
    __reg_t edx;
    __reg_t esi;
    __reg_t edi;
    __reg_t ebp;

    __reg_t interrupt;
    __reg_t error;

    // These are pushed by the cpu when an interrupt happens.
#  define INSTRUCTION_POINTER eip
    __reg_t eip;
    __reg_t cs;
    __reg_t eflags;

    // These are only valid if the interrupt came from Ring 3
#  define STACK_POINTER esp
    __reg_t esp;
    __reg_t ss;
#elif defined(__x86_64__)
    __reg_t rax;
    __reg_t rbx;
    __reg_t rcx;
    __reg_t rdx;
    __reg_t rsi;
    __reg_t rdi;
    __reg_t rbp;
    __reg_t r8;
    __reg_t r9;
    __reg_t r10;
    __reg_t r11;
    __reg_t r12;
    __reg_t r13;
    __reg_t r14;
    __reg_t r15;

    __reg_t interrupt;
    __reg_t error;

#  define INSTRUCTION_POINTER rip
    __reg_t rip;
    __reg_t cs;
    __reg_t rflags;
#  define STACK_POINTER rsp
    __reg_t rsp;
    __reg_t ss;
#else
#  error "InterruptContext is undefined for this architecture."
#endif
};

struct IrqHandler {
    void (*func)(void*, const InterruptContext*);
    void* user;
    IrqHandler* next;
};

namespace Interrupts {
extern uint8_t apicId;
extern bool hasApic;
extern int isaIrq[16];
extern int timerIrq;

void addIrqHandler(int irq, IrqHandler* handler);
int allocateIrq();
void disable();
void enable();
void initApic();
void initIoApic(paddr_t baseAddress, int interruptBase);
void initPic();
}

#endif
