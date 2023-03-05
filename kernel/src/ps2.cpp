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

#include <inttypes.h>
#include <cobalt/kernel/console.h>
#include <cobalt/kernel/interrupts.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/portio.h>
#include <cobalt/kernel/ps2keyboard.h>
#include <cobalt/kernel/ps2mouse.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

#define COMMAND_READ_CONFIG 0x20
#define COMMAND_WRITE_CONFIG 0x60
#define COMMAND_DISABLE_PORT2 0xA7
#define COMMAND_ENABLE_PORT2 0xA8
#define COMMAND_TEST_PORT2 0xA9
#define COMMAND_SELF_TEST 0xAA
#define COMMAND_TEST_PORT1 0xAB
#define COMMAND_DISABLE_PORT1 0xAD
#define COMMAND_ENABLE_PORT1 0xAE
#define COMMAND_SEND_TO_SECOND_PORT 0xD4

#define DEVICE_IDENTIFY 0xF2
#define DEVICE_ENABLE_SCANNING 0xF4
#define DEVICE_DISABLE_SCANNING 0xF5
#define DEVICE_RESET 0xFF

static void checkPort(bool secondPort);
static void irqHandler(void*, const InterruptContext* context);
static void sendPS2Command(uint8_t command);
static void sendPS2Command(uint8_t command, uint8_t data);
static uint8_t sendPS2CommandWithResponse(uint8_t command);

static PS2Device* ps2Device1;
static PS2Device* ps2Device2;

static IrqHandler handler1;
static IrqHandler handler2;

void PS2::initialize() {
    // Disable PS/2 devices
    sendPS2Command(COMMAND_DISABLE_PORT1);
    sendPS2Command(COMMAND_DISABLE_PORT2);

    // Flush the output buffer
    while (inb(PS2_STATUS_PORT) & 1) {
        inb(PS2_DATA_PORT);
    }

    // Configure the controller
    uint8_t config = sendPS2CommandWithResponse(COMMAND_READ_CONFIG);
    config &= ~((1 << 0) | (1 << 1));
    sendPS2Command(COMMAND_WRITE_CONFIG, config);

    uint8_t test = sendPS2CommandWithResponse(COMMAND_SELF_TEST);
    if (test != 0x55) {
        Log::printf("PS/2 self test failed (response = 0x%" PRIX8 ")\n", test);
        return;
    }

    bool dualChannel = false;

    if (config & (1 << 5)) {
        sendPS2Command(COMMAND_ENABLE_PORT2);
        if (!(sendPS2CommandWithResponse(COMMAND_READ_CONFIG) & (1 << 5))) {
            dualChannel = true;
        }
    }

    bool port1Exists = sendPS2CommandWithResponse(COMMAND_TEST_PORT1) == 0x00;
    bool port2Exists = dualChannel &&
            sendPS2CommandWithResponse(COMMAND_TEST_PORT2) == 0x00;

    if (!port1Exists && !port2Exists) {
        Log::printf("No usable PS/2 port found\n");
    }

    if (port1Exists) {
        sendPS2Command(COMMAND_ENABLE_PORT1);
    }

    if (port2Exists) {
        sendPS2Command(COMMAND_ENABLE_PORT2);
    }

    config = sendPS2CommandWithResponse(COMMAND_READ_CONFIG);
    if (port1Exists) {
        config |= (1 << 0);
    }

    if (port2Exists) {
        config |= (1 << 1);
    }

    sendPS2Command(COMMAND_WRITE_CONFIG, config);

    // Scan for connected devices
    if (port1Exists) {
        checkPort(false);
    }

    if (port2Exists) {
        checkPort(true);
    }

    if (ps2Device1) {
        PS2::sendDeviceCommand(false, DEVICE_ENABLE_SCANNING);
    }
    if (ps2Device2) {
        PS2::sendDeviceCommand(true, DEVICE_ENABLE_SCANNING);
    }
}

static void checkPort(bool secondPort) {
#ifdef BROKEN_PS2_EMULATION
    /* On some computers PS/2 emulation is completely broken. In this case we
       just assume that there is a keyboard connected to port 1 that just
       works without any additional initialization. */
    if (secondPort) return;
    PS2Keyboard* keyboard = xnew PS2Keyboard(false);
    keyboard->listener = (Console*) console;

    ps2Device1 = keyboard;
    handler1.func = irqHandler;
    Interrupts::addIrqHandler(Interrupts::isaIrq[1], &handler1);
#else
    // TODO: We should have a timeout in case get no response.
    if (PS2::sendDeviceCommand(secondPort, DEVICE_RESET) != 0xFA) return;
    if (PS2::readDataPort() != 0xAA) return;
    while (inb(PS2_STATUS_PORT) & 1) {
        PS2::readDataPort();
    }

    if (PS2::sendDeviceCommand(secondPort, DEVICE_DISABLE_SCANNING) != 0xFA) {
        return;
    }

    if (PS2::sendDeviceCommand(secondPort, DEVICE_IDENTIFY) != 0xFA) return;
    uint8_t id = PS2::readDataPort();

    PS2Device* device = nullptr;
    if (id == 0x00 || id == 0x03 || id == 0x04) {
        device = xnew PS2Mouse(secondPort);
    } else if (id == 0xAB) {
        id = PS2::readDataPort();

        if (id == 0x41 || id == 0xC1 || id == 0x83) {
            // The device identified itself as a keyboard
            PS2Keyboard* keyboard = xnew PS2Keyboard(secondPort);
            keyboard->listener = (Console*) console;
            device = keyboard;
        }
    }

    if (device) {
        if (!secondPort) {
            ps2Device1 = device;
            handler1.func = irqHandler;
            Interrupts::addIrqHandler(Interrupts::isaIrq[1], &handler1);
        } else {
            ps2Device2 = device;
            handler2.func = irqHandler;
            Interrupts::addIrqHandler(Interrupts::isaIrq[12], &handler2);
        }
    }
#endif
}

uint8_t PS2::sendDeviceCommand(bool secondPort, uint8_t command) {
    uint8_t response;
    for (int i = 0; i < 3; i++) {
        if (secondPort) {
            sendPS2Command(COMMAND_SEND_TO_SECOND_PORT);
        }
        while (inb(PS2_STATUS_PORT) & 2);
        outb(PS2_DATA_PORT, command);
        response = readDataPort();
        if (response != 0xFE) return response;
    }
    return response;
}

uint8_t PS2::sendDeviceCommand(bool secondPort, uint8_t command, uint8_t data,
        bool ackBeforeData) {
    uint8_t response;
    for (int i = 0; i < 3; i++) {
        if (secondPort) {
            sendPS2Command(COMMAND_SEND_TO_SECOND_PORT);
        }
        while (inb(PS2_STATUS_PORT) & 2);
        outb(PS2_DATA_PORT, command);
        if (ackBeforeData) {
            // The mouse sends 0xFA before receiving the date byte, the keyboard
            // does not.
            response = readDataPort();
            if (response == 0xFE) continue;
            if (response != 0xFA) return response;
        }
        if (secondPort) {
            sendPS2Command(COMMAND_SEND_TO_SECOND_PORT);
        }
        while (inb(PS2_STATUS_PORT) & 2);
        outb(PS2_DATA_PORT, data);
        response = readDataPort();
        if (response != 0xFE) return response;
    }
    return response;
}

static void irqHandler(void*, const InterruptContext* /*context*/) {
    // Unfortunately both mouse and keyboard data arrive at the same io port. We
    // are supposed to be able to distinguish them depending on which IRQ was
    // raised, but unfortunately this does not work reliably on buggy hardware
    // and emulators. Instead we read the PS/2 status register to check which
    // kind of data arrived. This seems to work better but is still not
    // completely reliable. We may occasionally receive keyboard bytes in the
    // mouse handler when keyboard and mouse are sending data at the same time.
    // The only way to fix this would be to write a USB driver.
    uint8_t status = inb(PS2_STATUS_PORT);
    if (status & 0x20) {
        ps2Device2->irqHandler();
    } else {
        ps2Device1->irqHandler();
    }
}

uint8_t PS2::readDataPort() {
    while (!(inb(PS2_STATUS_PORT) & 1));
    return inb(PS2_DATA_PORT);
}

static void sendPS2Command(uint8_t command) {
    while (inb(PS2_STATUS_PORT) & 2);
    outb(PS2_COMMAND_PORT, command);
}

static void sendPS2Command(uint8_t command, uint8_t data) {
    while (inb(PS2_STATUS_PORT) & 2);
    outb(PS2_COMMAND_PORT, command);
    while (inb(PS2_STATUS_PORT) & 2);
    outb(PS2_DATA_PORT, data);
}

static uint8_t sendPS2CommandWithResponse(uint8_t command) {
    while (inb(PS2_STATUS_PORT) & 2);
    outb(PS2_COMMAND_PORT, command);
    return PS2::readDataPort();
}
