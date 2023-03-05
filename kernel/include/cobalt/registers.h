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

#ifndef _COBALT_REGISTERS_H
#define _COBALT_REGISTERS_H

typedef unsigned long __reg_t;

typedef struct {
#ifdef __i386__
    __reg_t __eax;
    __reg_t __ebx;
    __reg_t __ecx;
    __reg_t __edx;
    __reg_t __esi;
    __reg_t __edi;
    __reg_t __ebp;
    __reg_t __eip;
    __reg_t __eflags;
    __reg_t __esp;
    __reg_t __tlsbase;
#elif defined(__x86_64__)
    __reg_t __rax;
    __reg_t __rbx;
    __reg_t __rcx;
    __reg_t __rdx;
    __reg_t __rsi;
    __reg_t __rdi;
    __reg_t __rbp;
    __reg_t __r8;
    __reg_t __r9;
    __reg_t __r10;
    __reg_t __r11;
    __reg_t __r12;
    __reg_t __r13;
    __reg_t __r14;
    __reg_t __r15;
    __reg_t __rip;
    __reg_t __rflags;
    __reg_t __rsp;
    __reg_t __tlsbase;
#else
#  error "__registers_t is undefined for this architecture."
#endif
} __registers_t;

#if defined(__i386__) || defined(__x86_64__)
typedef char __fpu_t[512] __attribute__((__aligned__(16)));
#else
#  error "__fpu_t is undefined for this architecture."
#endif

#endif
