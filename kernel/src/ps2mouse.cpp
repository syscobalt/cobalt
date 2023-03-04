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

#include <string.h>
#include <cobalt/kernel/devices.h>
#include <cobalt/kernel/interrupts.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/mouse.h>
#include <cobalt/kernel/portio.h>
#include <cobalt/kernel/ps2mouse.h>

#define MOUSE_GET_ID 0xF2
#define MOUSE_SET_SAMPLE_RATE 0xF3

#define PS2_MOUSE_LEFT_BUTTON (1 << 0)
#define PS2_MOUSE_RIGHT_BUTTON (1 << 1)
#define PS2_MOUSE_MIDDLE_BUTTON (1 << 2)
#define PS2_MOUSE_X_NEGATIVE (1 << 4)
#define PS2_MOUSE_Y_NEGATIVE (1 << 5)
#define PS2_MOUSE_X_OVERFLOW (1 << 6)
#define PS2_MOUSE_Y_OVERFLOW (1 << 7)

PS2Mouse::PS2Mouse(bool secondPort) : secondPort(secondPort) {
    hasMouseWheel = false;
    index = 0;
    packetsAvailable = 0;
    job.func = worker;
    job.context = this;

    PS2::sendDeviceCommand(secondPort, MOUSE_GET_ID);
    uint8_t id = PS2::readDataPort();
    if (id == 0x00) {
        // Execute the detection sequence for mice with a mouse wheel.
        PS2::sendDeviceCommand(secondPort, MOUSE_SET_SAMPLE_RATE, 200, true);
        PS2::sendDeviceCommand(secondPort, MOUSE_SET_SAMPLE_RATE, 100, true);
        PS2::sendDeviceCommand(secondPort, MOUSE_SET_SAMPLE_RATE, 80, true);
        PS2::sendDeviceCommand(secondPort, MOUSE_GET_ID);
        id = PS2::readDataPort();
        if (id == 0x03) {
            hasMouseWheel = true;
        }
    }
    PS2::sendDeviceCommand(secondPort, MOUSE_SET_SAMPLE_RATE, 40, true);
    Log::printf("PS/2 mouse found\n");
}

void PS2Mouse::irqHandler() {
    buffer[index++] = inb(0x60);
    if (index == 1 && !(buffer[0] & (1 << 3))) {
        // This is an invalid first byte.
        index = 0;
    }

    if (index == 4 || (index == 3 && !hasMouseWheel)) {
        index = 0;

        if (buffer[0] & (PS2_MOUSE_X_OVERFLOW | PS2_MOUSE_Y_OVERFLOW)) {
            // Overflow, discard the packet.
        } else {
            mouse_data data;
            data.mouse_flags = 0;
            if (buffer[0] & PS2_MOUSE_LEFT_BUTTON) {
                data.mouse_flags |= MOUSE_LEFT;
            }
            if (buffer[0] & PS2_MOUSE_RIGHT_BUTTON) {
                data.mouse_flags |= MOUSE_RIGHT;
            }
            if (buffer[0] & PS2_MOUSE_MIDDLE_BUTTON) {
                data.mouse_flags |= MOUSE_MIDDLE;
            }

            if (hasMouseWheel && (buffer[3] & 0xF) == 0x1) {
                data.mouse_flags |= MOUSE_SCROLL_DOWN;
            } else if (hasMouseWheel && (buffer[3] & 0xF) == 0xF) {
                data.mouse_flags |= MOUSE_SCROLL_UP;
            }

            if (buffer[0] & PS2_MOUSE_X_NEGATIVE) {
                data.mouse_x = -0x100 + buffer[1];
            } else {
                data.mouse_x = buffer[1];
            }

            if (buffer[0] & PS2_MOUSE_Y_NEGATIVE) {
                data.mouse_y = 0x100 - buffer[2];
            } else {
                data.mouse_y = -buffer[2];
            }

            if (packetsAvailable == sizeof(packetBuffer) /
                    sizeof(packetBuffer[0])) {
                return;
            }
            packetBuffer[packetsAvailable] = data;
            packetsAvailable++;
            if (packetsAvailable == 1) {
                WorkerThread::addJob(&job);
            }
        }
    }
}

void PS2Mouse::work() {
    mouse_data buf[128];

    Interrupts::disable();
    size_t entries = packetsAvailable;
    memcpy(buf, packetBuffer, entries * sizeof(mouse_data));
    packetsAvailable = 0;
    Interrupts::enable();

    // During boot the mouse device might not have been created yet.
    if (!mouseDevice) return;

    for (size_t i = 0; i < entries; i++) {
        mouseDevice->addPacket(buf[i]);
    }
}

void PS2Mouse::worker(void* self) {
    PS2Mouse* mouse = (PS2Mouse*) self;
    mouse->work();
}
