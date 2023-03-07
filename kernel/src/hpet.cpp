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

#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/clock.h>
#include <cobalt/kernel/hpet.h>
#include <cobalt/kernel/interrupts.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/panic.h>

#define HPET_CAP_64BIT (1 << 13)
#define HPET_CAP_LEGACY_REPLACEMENT (1 << 15)

#define HPET_CONFIG_ENABLED (1 << 0)
#define HPET_CONFIG_LEGACY_REPLACEMENT (1 << 1)

#define TIMER_CONFIG_LEVEL_TRIGGERED (1 << 1)
#define TIMER_CONFIG_ENABLED (1 << 2)
#define TIMER_CONFIG_PERIODIC (1 << 3)
#define TIMER_CONFIG_SUPPORTS_PERIODIC (1 << 4)
#define TIMER_CONFIG_SUPPORTS_64BIT (1 << 5)
#define TIMER_CONFIG_SET_ACCUMULATOR (1 << 6)
#define TIMER_CONFIG_FSB (1 << 14)
#define TIMER_CONFIG_SUPPORTS_FSB (1 << 15)

static unsigned long nanoseconds;
static IrqHandler handler;

static void irqHandler(void*, const InterruptContext* context) {
    Clock::onTick(context->cs != 0x8, nanoseconds);
}

void Hpet::initialize(paddr_t baseAddress) {
    vaddr_t mapping;
    size_t mapSize;

    vaddr_t mapped = kernelSpace->mapUnaligned(baseAddress, 1024,
            PROT_READ | PROT_WRITE, mapping, mapSize);
    if (!mapped) PANIC("Failed to map HPET");

    uint32_t capabilites = *(volatile uint32_t*) mapped;
    bool legacyReplacementSupported = capabilites & HPET_CAP_LEGACY_REPLACEMENT;
    bool has64Bit = capabilites & HPET_CAP_64BIT;

    volatile uint32_t* timer0ConfigReg = (volatile uint32_t*) (mapped + 0x100);
    uint32_t timer0Config = *timer0ConfigReg;
    bool fsbSupported = timer0Config & TIMER_CONFIG_SUPPORTS_FSB;

    uint32_t period = *(volatile uint32_t*) (mapped + 0x4);
    uint64_t count = 1000000000000ULL / period;
    nanoseconds = count * period / 1000000;
    has64Bit = has64Bit && (timer0Config & TIMER_CONFIG_SUPPORTS_64BIT);
    bool periodic = timer0Config & TIMER_CONFIG_SUPPORTS_PERIODIC;

    if (!periodic) {
        Log::printf("HPET does not support periodic mode\n");
        kernelSpace->unmapPhysical(mapping, mapSize);
        return;
    }

    timer0Config |= TIMER_CONFIG_SET_ACCUMULATOR;
    timer0Config |= TIMER_CONFIG_PERIODIC;
    timer0Config |= TIMER_CONFIG_ENABLED;
    timer0Config &= ~TIMER_CONFIG_LEVEL_TRIGGERED;

    volatile uint32_t* generalConfigReg = (volatile uint32_t*) (mapped + 0x10);
    uint32_t generalConfig = *generalConfigReg;
    int irq = -1;

    if (fsbSupported) {
        timer0Config |= TIMER_CONFIG_FSB;
        volatile uint32_t* timer0FsbValue =
                (volatile uint32_t*) (mapped + 0x110);
        volatile uint32_t* timer0FsbAddress =
                (volatile uint32_t*) (mapped + 0x114);

        irq = Interrupts::allocateIrq();
        if (irq < 0) PANIC("Failed to allocate IRQ");

        *timer0FsbAddress = 0xFEE00000 | (Interrupts::apicId << 12);
        *timer0FsbValue = irq - 16 + 51;

        generalConfig &= ~HPET_CONFIG_LEGACY_REPLACEMENT;
    } else if (legacyReplacementSupported) {
        generalConfig |= HPET_CONFIG_LEGACY_REPLACEMENT;
        irq = Interrupts::isaIrq[0];
    } else {
        uint32_t routes = *(volatile uint32_t*) (mapped + 0x104);

        // Try to find an IRQ that does not conflict with ISA IRQs.
        for (int i = 16; i < 32; i++) {
            if (routes & (1U << i)) {
                irq = i;
                break;
            }
        }

        if (irq < 0) {
            Log::printf("HPET has no supported interrupt routing.\n");
            kernelSpace->unmapPhysical(mapping, mapSize);
            return;
        }

        timer0Config = (timer0Config & ~0x3E00) | (irq << 9);
        generalConfig &= ~HPET_CONFIG_LEGACY_REPLACEMENT;
    }

    Log::printf("HPET is using IRQ%d\n", irq);
    *timer0ConfigReg = timer0Config;

    volatile uint32_t* timer0ComparatorLow =
            (volatile uint32_t*) (mapped + 0x108);
    volatile uint32_t* timer0ComparatorHigh =
            (volatile uint32_t*) (mapped + 0x10C);
    *timer0ComparatorLow = count & 0xFFFFFFFF;
    *timer0ConfigReg = timer0Config;
    *timer0ComparatorHigh = count >> 32;

    volatile uint32_t* mainCounterLow = (volatile uint32_t*) (mapped + 0xF0);
    volatile uint32_t* mainCounterHigh = (volatile uint32_t*) (mapped + 0xF4);
    *mainCounterLow = 0;
    *mainCounterHigh = 0;

    handler.func = irqHandler;
    Interrupts::addIrqHandler(irq, &handler);
    Interrupts::timerIrq = irq;

    generalConfig |= HPET_CONFIG_ENABLED;
    *generalConfigReg = generalConfig;

    kernelSpace->unmapPhysical(mapping, mapSize);
}
