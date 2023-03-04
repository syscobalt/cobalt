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

#include <stdio.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/console.h>
#include <cobalt/kernel/display.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/multiboot2.h>

static size_t callback(void*, const char* s, size_t nBytes);

// To avoid early memory allocations we statically allocate enough space for
// the display.
char displayBuf[sizeof(Display)];

void Log::earlyInitialize(const multiboot_info* multiboot) {
    uintptr_t p = (uintptr_t) multiboot + 8;
    const multiboot_tag* tag;

    while (true) {
        tag = (const multiboot_tag*) p;
        if (tag->type == MULTIBOOT_TAG_TYPE_END ||
                tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
            break;
        }

        p = ALIGNUP(p + tag->size, 8);
    }

    const multiboot_tag_framebuffer* fbTag =
            (const multiboot_tag_framebuffer*) tag;

    if (tag->type == MULTIBOOT_TAG_TYPE_END ||
            fbTag->framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT) {
        vaddr_t videoMapped = kernelSpace->mapPhysical(0xB8000, PAGESIZE,
                PROT_READ | PROT_WRITE);
        video_mode mode;
        mode.video_bpp = 0;
        mode.video_height = 25;
        mode.video_width = 80;
        console->display = new (displayBuf) Display(mode,
                (char*) videoMapped, 160);
        console->updateDisplaySize();
    } else if (fbTag->framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_RGB &&
            (fbTag->framebuffer_bpp == 24 || fbTag->framebuffer_bpp == 32)) {
        vaddr_t lfbMapping;
        size_t mapSize;
        vaddr_t lfb = kernelSpace->mapUnaligned(fbTag->framebuffer_addr,
                fbTag->framebuffer_height * fbTag->framebuffer_pitch,
                PROT_READ | PROT_WRITE | PROT_WRITE_COMBINING, lfbMapping,
                mapSize);
        if (!lfb) {
            // This shouldn't fail in practise as enough memory should be
            // available.
            asm ("hlt");
        }

        video_mode mode;
        mode.video_bpp = fbTag->framebuffer_bpp;
        mode.video_height = fbTag->framebuffer_height;
        mode.video_width = fbTag->framebuffer_width;
        console->display = new (displayBuf) Display(mode, (char*) lfb,
                fbTag->framebuffer_pitch);
        console->updateDisplaySize();
    } else {
        // Without any usable display we cannot do anything.
        while (true) asm volatile ("cli; hlt");
    }
}

void Log::initialize() {
    console->display->initialize();
}

void Log::printf(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    Log::vprintf(format, ap);
    va_end(ap);
}

void Log::vprintf(const char* format, va_list ap) {
    vcbprintf(nullptr, callback, format, ap);
}

static size_t callback(void*, const char* s, size_t nBytes) {
    return console->write(s, nBytes, 0);
}
