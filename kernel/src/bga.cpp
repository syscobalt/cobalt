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

#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/bga.h>
#include <cobalt/kernel/pci.h>
#include <cobalt/kernel/portio.h>

#define VBE_DISPI_IOPORT_INDEX 0x1CE
#define VBE_DISPI_IOPORT_DATA 0x1CF

#define VBE_DISPI_INDEX_ID 0
#define VBE_DISPI_INDEX_XRES 1
#define VBE_DISPI_INDEX_YRES 2
#define VBE_DISPI_INDEX_BPP 3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_INDEX_BANK 5
#define VBE_DISPI_INDEX_VIRT_WIDTH 6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET 8
#define VBE_DISPI_INDEX_Y_OFFSET 9

#define VBE_DISPI_ENABLE 0x01
#define VBE_DISPI_GETCAPS 0x02
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x80

static uint16_t readRegister(uint16_t index) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    return inw(VBE_DISPI_IOPORT_DATA);
}

static void writeRegister(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

void BgaDevice::initialize(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t version = readRegister(VBE_DISPI_INDEX_ID);

    if (version == 0xB0C0) {
        // Work around a bug in QEMU where it supports 0xB0C5 but but does not
        // allow the ID register to be set to that value and therefore defaults
        // to 0xB0C0.
        writeRegister(VBE_DISPI_INDEX_ID, 0xB0C4);
        version = readRegister(VBE_DISPI_INDEX_ID);
    }
    if (version < 0xB0C3) return;

   graphicsDriver = xnew BgaDevice(version, bus, device, function);
}

BgaDevice::BgaDevice(uint16_t version, uint8_t bus, uint8_t device,
        uint8_t function) : version(version), bus(bus), device(device),
        function(function) {
    framebuffer = 0;

    uint16_t enable = readRegister(VBE_DISPI_INDEX_ENABLE);
    writeRegister(VBE_DISPI_INDEX_ENABLE, enable | VBE_DISPI_GETCAPS);
    maxX = readRegister(VBE_DISPI_INDEX_XRES);
    maxY = readRegister(VBE_DISPI_INDEX_YRES);
    writeRegister(VBE_DISPI_INDEX_ENABLE, enable);
}

bool BgaDevice::isSupportedMode(video_mode mode) {
    if (mode.video_bpp != 24 && mode.video_bpp != 32) return false;
    if (mode.video_width > maxX || mode.video_height > maxY) return false;
    size_t neededSize = mode.video_width * mode.video_height *
            (mode.video_bpp / 8);
    size_t fbSize = version >= 0xB0C5 ? 16 * 1024 * 1024 :
                version == 0xB0C4 ? 8 * 1024 * 1024 :
                4 * 1024 * 1024;
    if (neededSize > fbSize) return false;
    if (mode.video_width < 320 || mode.video_height < 240) return false;

    return true;
}

vaddr_t BgaDevice::setVideoMode(video_mode* mode) {
    if (!framebuffer) {
        paddr_t physicalFramebufferAddress = Pci::readConfig(bus, device,
                function, offsetof(PciHeader, bar0)) & ~0xF;
        size_t fbSize = version >= 0xB0C5 ? 16 * 1024 * 1024 :
                version == 0xB0C4 ? 8 * 1024 * 1024 :
                4 * 1024 * 1024;
        framebuffer = kernelSpace->mapPhysical(physicalFramebufferAddress,
                fbSize, PROT_READ | PROT_WRITE | PROT_WRITE_COMBINING);
        if (!framebuffer) return 0;
    }
    writeRegister(VBE_DISPI_INDEX_ENABLE, 0);
    writeRegister(VBE_DISPI_INDEX_XRES, mode->video_width);
    writeRegister(VBE_DISPI_INDEX_YRES, mode->video_height);
    writeRegister(VBE_DISPI_INDEX_BPP, mode->video_bpp);
    writeRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLE |
            VBE_DISPI_LFB_ENABLED);

    mode->video_width = readRegister(VBE_DISPI_INDEX_XRES);
    mode->video_height = readRegister(VBE_DISPI_INDEX_YRES);
    mode->video_bpp = readRegister(VBE_DISPI_INDEX_BPP);

    return framebuffer;
}
