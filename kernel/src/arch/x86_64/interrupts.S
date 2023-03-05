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

.section .text

.macro isr no
.global isr_\no
isr_\no:
    push $0 # no error code
    push $\no
    jmp commonHandler
.endm

.macro isr_error_code no
.global isr_\no
isr_\no:
    # an error code was already pushed
    push $\no
    jmp commonHandler
.endm

.type commonHandler, @function
commonHandler:
    cld

    # Push registers
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

    # Switch to kernel data segment
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es

    mov %rsp, %rdi
    call handleInterrupt
    mov %rax, %rsp

    # Check whether signals are pending.
    mov signalPending, %rbx
    test %rbx, %rbx
    jz 1f

    # Don't handle signals when returning to kernelspace.
    mov 144(%rsp), %rbx # cs
    cmp $0x8, %rbx
    je 1f

    mov %rsp, %rdi
    call handleSignal
    mov %rax, %rsp

    # Switch back to user data segment
1:  mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es

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

    # Remove error code and interrupt number from stack
    add $16, %rsp

    iretq
.size commonHandler, . - commonHandler

# CPU Exceptions
isr 0             # Devide-by-zero Error
isr 1             # Debug
isr 2             # Non-maskable Interrupt
isr 3             # Breakpoint
isr 4             # Overflow
isr 5             # Bound Range Exceeded
isr 6             # Invalid Opcode
isr 7             # Device Not Available
isr_error_code 8  # Double Fault
isr 9             # Coprocessor Segment Overrun
isr_error_code 10 # Invalid TSS
isr_error_code 11 # Segment Not Present
isr_error_code 12 # Stack-Segement Fault
isr_error_code 13 # General Protection Fault
isr_error_code 14 # Page Fault
isr 15            # Reserved
isr 16            # x87 Floating-Point Exception
isr_error_code 17 # Alignment Check
isr 18            # Machine Check
isr 19            # SIMD Floating-Point Exception
isr 20            # Virtualization Exception

# IRQ0 - IRQ15
isr 32
isr 33
isr 34
isr 35
isr 36
isr 37
isr 38
isr 39
isr 40
isr 41
isr 42
isr 43
isr 44
isr 45
isr 46
isr 47

#isr 48 # Syscall
isr 49 # Schedule
isr 50 # sigreturn

# IRQ16 - IRQ219, Spurious Interrupt
.altmacro
.set i, 51
.rept 255 - 50
isr %i
.set i, i+1
.endr

.global beginSigreturn
beginSigreturn:
    # This code is mapped in all user address spaces. When a userspace
    # program returns from a signal handler it will return to this address and
    # and then perform a sigreturn.
    int $0x32
.global endSigreturn
endSigreturn:
