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

#ifndef KERNEL_BGA_H
#define KERNEL_BGA_H

#include <cobalt/kernel/display.h>

class BgaDevice : public GraphicsDriver {
public:
    BgaDevice(uint16_t version, uint8_t bus, uint8_t device, uint8_t function);
    bool isSupportedMode(video_mode mode) override;
    vaddr_t setVideoMode(video_mode* mode) override;
public:
    static void initialize(uint8_t bus, uint8_t device, uint8_t function);
private:
    vaddr_t framebuffer;
    uint16_t version;
    uint16_t maxX;
    uint16_t maxY;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};

#endif
