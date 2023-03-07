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

.section .text
.global syscallHandler
.type syscallHandler, @function
syscallHandler:
    cld

    mov $0x10, %r10w
    mov %r10w, %ds
    mov %r10w, %es

    sub $8, %rsp

    push %rdi
    push %rsi
    push %rdx
    push %rcx
    push %r8
    push %r9

    mov %rax, %rdi
    call getSyscallHandler

    pop %r9
    pop %r8
    pop %rcx
    pop %rdx
    pop %rsi
    pop %rdi

    mov __errno_location, %r11
    movl $0, (%r11)
    call *%rax

    add $8, %rsp

    mov __errno_location, %r11
    mov (%r11), %edi

    # Check whether signals are pending.
    mov signalPending, %r10
    test %r10, %r10
    jnz 2f

    mov $0x23, %r10w
    mov %r10w, %ds
    mov %r10w, %es

1:  iretq

# Fake an InterruptContext so that we can call handleSignal.
2:  sub $16, %rsp
    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rbp
    push %rdi
    push %rsi
    push %rdx
    push %rcx
    push %rbx
    push %rax

    mov %rsp, %rdi
    call handleSignal
    mov %rax, %rsp

    pop %rax
    pop %rbx
    pop %rcx
    pop %rdx
    pop %rsi
    pop %rdi
    pop %rbp
    pop %r8
    pop %r9
    pop %r10
    pop %r11
    pop %r12
    pop %r13
    pop %r14
    pop %r15
    add $16, %rsp

    jmp 1b
.size syscallHandler, . - syscallHandler
