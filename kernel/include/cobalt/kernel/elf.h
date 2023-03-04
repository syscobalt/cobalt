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

#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

#include <stdint.h>

typedef uintptr_t Elf_Addr;
#ifdef __i386__
typedef uint32_t Elf_Off;
#elif defined(__x86_64__)
typedef uint64_t Elf_Off;
#endif

struct ElfHeader {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    Elf_Addr e_entry;
    Elf_Off e_phoff;
    Elf_Off e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct ProgramHeader32 {
    uint32_t p_type;
    Elf_Off p_offset;
    Elf_Addr p_vaddr;
    Elf_Addr p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
};

struct ProgramHeader64 {
    uint32_t p_type;
    uint32_t p_flags;
    Elf_Off p_offset;
    Elf_Addr p_vaddr;
    Elf_Addr p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

#ifdef __i386__
typedef ProgramHeader32 ProgramHeader;
#elif defined(__x86_64__)
typedef ProgramHeader64 ProgramHeader;
#endif

#define PT_LOAD 1
#define PT_TLS 7
#define PF_X 1
#define PF_W 2
#define PF_R 4

#endif
