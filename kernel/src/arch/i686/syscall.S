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

.section .text
.global syscallHandler
.type syscallHandler, @function
syscallHandler:
    cld

    mov %esp, %ebp
    and $(~0xFF), %esp
    sub $12, %esp
    push %edi

    push %esi
    push %edx
    push %ecx
    push %ebx

    sub $12, %esp
    push %eax

    mov $0x10, %cx
    mov %cx, %ds
    mov %cx, %es

    call getSyscallHandler
    add $16, %esp
    mov __errno_location, %ecx
    movl $0, (%ecx)

    call *%eax
    mov %ebp, %esp

    # Check whether signals are pending.
    mov signalPending, %ecx
    test %ecx, %ecx
    jnz 2f

    mov $0x23, %cx
    mov %cx, %ds
    mov %cx, %es

    mov __errno_location, %ecx
    mov (%ecx), %ecx

1:  iret

    # Fake an InterruptContext so that we can call handleSignal.
2:  sub $8, %esp
    push %ebp
    push %edi
    push %esi
    push %edx
    mov __errno_location, %ecx
    pushl (%ecx)
    push %ebx
    push %eax

    mov %esp, %eax
    and $(~0xFF), %esp # Align the stack
    sub $12, %esp
    push %eax

    call handleSignal
    mov %eax, %esp

    pop %eax
    pop %ebx
    pop %ecx
    pop %edx
    pop %esi
    pop %edi
    pop %ebp
    add $8, %esp

    jmp 1b
.size syscallHandler, . - syscallHandler
