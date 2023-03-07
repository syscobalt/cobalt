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

#include <cobalt/kernel/clock.h>
#include <cobalt/kernel/interrupts.h>
#include <cobalt/kernel/pit.h>
#include <cobalt/kernel/portio.h>

#define PIT_FREQUENCY 1193182 // Hz

#define PIT_PORT_CHANNEL0 0x40
#define PIT_PORT_MODE 0x43

#define PIT_MODE_RATE_GENERATOR 0x4
#define PIT_MODE_LOBYTE_HIBYTE 0x30

// This should fire the timer approximately every millisecond.
static const unsigned int frequency = 1000;
static const uint16_t divider = PIT_FREQUENCY / frequency;
static const unsigned long nanoseconds = 1000000000L / PIT_FREQUENCY * divider;

static void irqHandler(void*, const InterruptContext* context);
static IrqHandler handler;

void Pit::initialize() {
    if (Interrupts::timerIrq != -1) return;

    handler.func = irqHandler;
    Interrupts::addIrqHandler(Interrupts::isaIrq[0], &handler);
    Interrupts::timerIrq = Interrupts::isaIrq[0];

    outb(PIT_PORT_MODE, PIT_MODE_RATE_GENERATOR | PIT_MODE_LOBYTE_HIBYTE);
    outb(PIT_PORT_CHANNEL0, divider & 0xFF);
    outb(PIT_PORT_CHANNEL0, (divider >> 8) & 0xFF);
}

static void irqHandler(void*, const InterruptContext* context) {
    Clock::onTick(context->cs != 0x8, nanoseconds);
}
