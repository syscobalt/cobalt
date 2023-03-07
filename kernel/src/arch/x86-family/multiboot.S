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

#define MULTIBOOT_MAGIC 0xE85250D6
#define MULTIBOOT_ARCH 0
#define MULTIBOOT_HEADER_LENGTH (multibootEnd - multibootStart)
#define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_ARCH + \
        MULTIBOOT_HEADER_LENGTH)

# Choose preferred video settings. The bootloader is still allowed to choose a
# different video mode.
#define PREFERRED_WIDTH 1024
#define PREFERRED_HEIGHT 768

.section multiboot
.align 8
multibootStart:
.long MULTIBOOT_MAGIC
.long MULTIBOOT_ARCH
.long MULTIBOOT_HEADER_LENGTH
.long MULTIBOOT_CHECKSUM

# Request information from the bootloader.
.align 8
.word 1
.word 0
.long 28 # Size
.long 3 # Modules
.long 6 # Memory map
.long 8 # Framebuffer info
.long 14 # RSDP (ACPI 1.0)
.long 15 # RSDP (ACPI 2.0+)

# Request a framebuffer.
.align 8
.word 5
.word 0
.long 20
.long PREFERRED_WIDTH
.long PREFERRED_HEIGHT
.long 32 # bpp

# Request modules to be page aligned.
.align 8
.word 6
.word 0
.long 8

# End of tags.
.align 8
.word 0
.word 0
.long 8
multibootEnd:
