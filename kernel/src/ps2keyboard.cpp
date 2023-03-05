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

#include <string.h>
#include <cobalt/kernel/interrupts.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/portio.h>
#include <cobalt/kernel/ps2keyboard.h>

#define KEYBOARD_SET_LED 0xED

PS2Keyboard::PS2Keyboard(bool secondPort) : secondPort(secondPort) {
    listener = nullptr;
    available = 0;
    job.func = worker;
    job.context = this;
    Log::printf("PS/2 Keyboard found.\n");
}

enum {
    STATE_NORMAL,
    STATE_ESCAPED,
};

static int state = STATE_NORMAL;
static uint8_t ledState = 0;

void PS2Keyboard::irqHandler() {
    uint8_t data = inb(0x60);
    int keycode;

    if (data == 0xFA || data == 0xFE) {
        return;
    } else if (data == 0xE0) {
        state = STATE_ESCAPED;
    } else {
        if (state == STATE_NORMAL) {
            keycode = data & 0x7F;
        } else {
            keycode = data | 0x80;
            state = STATE_NORMAL;
        }
        bool released = data & 0x80;

        handleKey(released ? -keycode : keycode);
    }
}

void PS2Keyboard::handleKey(int keycode) {
    uint8_t newLed = ledState;
    if (keycode == 0x45) { // NumLock
        newLed ^= 2;
    } else if (keycode == 0x3A) { // CapsLock
        newLed ^= 4;
    } else if (keycode == 0x46) { // ScrollLock
        newLed ^= 1;
    }

    if (newLed != ledState) {
        ledState = newLed;

        PS2::sendDeviceCommand(secondPort, KEYBOARD_SET_LED, ledState, false);
    }

    if (available == sizeof(buffer) / sizeof(buffer[0])) {
        return;
    }

    buffer[available] = keycode;
    available++;

    if (available == 1) {
        WorkerThread::addJob(&job);
    }
}

void PS2Keyboard::work() {
    int buf[128];

    Interrupts::disable();
    size_t entries = available;
    memcpy(buf, buffer, entries * sizeof(int));
    available = 0;
    Interrupts::enable();

    for (size_t i = 0; i < entries; i++) {
        if (listener) {
            listener->onKeyboardEvent(buf[i]);
        }
    }
}

void PS2Keyboard::worker(void* self) {
    PS2Keyboard* keyboard = (PS2Keyboard*) self;
    keyboard->work();
}
