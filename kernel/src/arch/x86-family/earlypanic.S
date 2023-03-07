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

#include <cobalt/kernel/arch.h>

.code32
.section bootstrap_text, "ax"
.global earlyPanic
.type earlyPanic, @function
earlyPanic:
    # Tries to print an error message to the screen and halts. The error message
    # is expected in eax and a pointer to the multiboot structure in esi.
    # We cannot run any C code because initialization was not completed.
    add $8, %esi
1:  cmp $8, (%esi)
    je 1f
    cmp $0, (%esi)
    je text
    add 4(%esi), %esi
    add $7, %esi
    and $-8, %esi
    jmp 1b

1:  cmpb $1, 29(%esi)
    je graphics
    cmpb $2, 29(%esi)
    je text
    # We cannot print anything when the bootloader chooses some weird mode.
    jmp halt

text:
    mov $0xB8000, %ebx
1:  mov (%eax), %cl
    test %cl, %cl
    jz halt
    mov %cl, (%ebx)
    movb $0x4F, 1(%ebx) # white on red
    inc %eax
    add $2, %ebx
    jmp 1b

graphics:
    # We only support 24 and 32 bpp.
    cmpb $24, 28(%esi)
    je 1f
    cmpb $32, 28(%esi)
    jne halt

1:  xor %ebx, %ebx
    mov 28(%esi), %bl
    mov %ebx, %esp # bpp
    shr $3, %ebx # bpp / 8

    # Get the LFB address.
    mov 8(%esi), %edi
    # Get the physical address of the vgafont.
    mov $(vgafont - KERNEL_VIRTUAL), %ebp

1:  xor %ecx, %ecx
    mov (%eax), %cl
    test %cl, %cl
    jz halt
    shl $4, %cx
    add %ebp, %ecx
    # ecx now contains the address of the correct glyph in the vgafont.

2:  mov $0x10000, %edx
    movb (%ecx), %dl
3:  test $0x80, %dl
    jz 4f
    # White foreground
    movb $0xFF, 0(%edi)
    movb $0xFF, 1(%edi)
    movb $0xFF, 2(%edi)
    jmp 5f
    # Red background
4:  movb $0x00, 0(%edi)
    movb $0x00, 1(%edi)
    movb $0xAA, 2(%edi)
5:  add %ebx, %edi
    shl %edx
    test $0xFF0000, %edx
    jnz 3b

    # Fill the one pixel wide space between characters.
    movb $0x00, 0(%edi)
    movb $0x00, 1(%edi)
    movb $0xAA, 2(%edi)

    sub %esp, %edi
    add 16(%esi), %edi
    inc %ecx
    mov %ebp, %edx
    xor %ecx, %edx
    test $0xF, %edx
    jnz 2b

    mov 16(%esi), %edx
    shl $4, %edx
    sub %edx, %edi
    add %esp, %edi
    add %ebx, %edi

    inc %eax
    jmp 1b
halt:
    hlt
.size earlyPanic, . - earlyPanic
