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

#include <cobalt/kernel/ahci.h>
#include <cobalt/kernel/ata.h>
#include <cobalt/kernel/bga.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/pci.h>
#include <cobalt/kernel/portio.h>
#include <cobalt/kernel/virtualbox.h>

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

#define PCI_ADDRESS_ENABLE (1 << 31)
#define PCI_HEADER_MULTIFUNCTION 0x80

#define PCI_STATUS_CAPABILITY_LIST (1 << 4)
#define PCI_CAP_MSI 0x5
#define PCI_MSI_ENABLE (1 << 0)
#define PCI_MSI_64BIT (1 << 7)

struct PciBridgeHeader {
    uint16_t vendorId;
    uint16_t deviceId;
    uint16_t command;
    uint16_t status;
    uint8_t revisionId;
    uint8_t progIf;
    uint8_t subclass;
    uint8_t classCode;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t bist;
    uint32_t bar0;
    uint32_t bar1;
    uint8_t primaryBusNumber;
    uint8_t secondaryBusNumber;
    // Other members omitted.
};

static void checkBus(uint8_t bus);

int Pci::getIrq(unsigned int bus, unsigned int device, unsigned int function) {
    if (!Interrupts::hasApic) {
        uint8_t interruptLine = readConfig(bus, device, function,
                offsetof(PciHeader, interruptLine));
        return interruptLine;
    }

    // Check whether the device supports MSI.
    uint16_t status = readConfig(bus, device, function,
            offsetof(PciHeader, status));

    if (status & PCI_STATUS_CAPABILITY_LIST) {
        uint8_t capability = readConfig(bus, device, function,
                offsetof(PciHeader, capabilitiesPointer)) & 0xFC;
        while (capability) {
            uint16_t header = readConfig(bus, device, function, capability);

            if ((header & 0xFF) == PCI_CAP_MSI) {
                uint16_t messageControl = readConfig(bus, device, function,
                        capability + 2);
                bool has64Bit = messageControl & PCI_MSI_64BIT;

                int irq = Interrupts::allocateIrq();
                if (irq < 0) return -1;

                uint32_t address = 0xFEE00000 | (Interrupts::apicId << 12);
                uint16_t value = irq - 16 + 51;

                writeConfig(bus, device, function, capability + 4, address);
                if (has64Bit) {
                    writeConfig(bus, device, function, capability + 8, 0);
                    uint32_t config = readConfig(bus, device, function,
                            capability + 12);
                    config = (config & 0xFFFF0000) | value;
                    writeConfig(bus, device, function, capability + 12, config);
                } else {
                    uint32_t config = readConfig(bus, device, function,
                            capability + 8);
                    config = (config & 0xFFFF0000) | value;
                    writeConfig(bus, device, function, capability + 8, config);
                }

                messageControl &= ~0x70;
                messageControl |= PCI_MSI_ENABLE;
                uint32_t config = (messageControl << 16) | header;
                writeConfig(bus, device, function, capability, config);

                return irq;
            }

            capability = (header >> 8) & 0xFC;
        }
    }

    return -1;
}

uint32_t Pci::readConfig(unsigned int bus, unsigned int device,
        unsigned int function, unsigned int offset) {
    uint32_t address = PCI_ADDRESS_ENABLE | bus << 16 | device << 11 |
            function << 8 | (offset & 0xFC);
    outl(CONFIG_ADDRESS, address);
    uint32_t word = inl(CONFIG_DATA);
    return word >> (8 * (offset & 0x3));
}

void Pci::writeConfig(unsigned int bus, unsigned int device,
        unsigned int function, unsigned int offset, uint32_t value) {
    uint32_t address = PCI_ADDRESS_ENABLE | bus << 16 | device << 11 |
            function << 8 | offset;
    outl(CONFIG_ADDRESS, address);
    outl(CONFIG_DATA, value);
}

static void checkFunction(uint8_t bus, uint8_t device, uint8_t function,
        uint16_t vendor) {
    uint16_t deviceId = Pci::readConfig(bus, device, function,
            offsetof(PciHeader, deviceId));
    uint8_t classCode = Pci::readConfig(bus, device, function,
            offsetof(PciHeader, classCode));
    uint8_t subclass = Pci::readConfig(bus, device, function,
            offsetof(PciHeader, subclass));
#ifdef PCI_DEBUG
    Log::printf("%u/%u/%u: vendor %X, device %X, class %X, subclass %X\n",
            bus, device, function, vendor, deviceId, classCode, subclass);
#endif

    // Handle devices for which we have a driver.
    if ((vendor == 0x1234 && deviceId == 0x1111) ||
            (vendor == 0x80EE && deviceId == 0xBEEF)) {
        BgaDevice::initialize(bus, device, function);
    }

    if (vendor == 0x80EE && deviceId == 0xCAFE) {
        VirtualBox::initialize(bus, device, function);
    }

    if (classCode == 0x01 && subclass == 0x01) {
        AtaController::initialize(bus, device, function);
    }

    if (classCode == 0x01 && subclass == 0x06) {
        Ahci::initialize(bus, device, function);
    }

    // Scan PCI bridges for more devices.
    if (classCode == 0x06 && subclass == 0x04) {
        uint8_t secondaryBus = Pci::readConfig(bus, device, function,
                offsetof(PciBridgeHeader, secondaryBusNumber));
        checkBus(secondaryBus);
    }
}

static void checkDevice(uint8_t bus, uint8_t device) {
    uint16_t vendorId = Pci::readConfig(bus, device, 0,
            offsetof(PciHeader, vendorId));
    if (vendorId == 0xFFFF) return;
    checkFunction(bus, device, 0, vendorId);

    uint8_t headerType = Pci::readConfig(bus, device, 0,
            offsetof(PciHeader, headerType));
    if (headerType & PCI_HEADER_MULTIFUNCTION) {
        for (uint8_t i = 1; i < 8; i++) {
            vendorId = Pci::readConfig(bus, device, i,
                    offsetof(PciHeader, vendorId));
            if (vendorId == 0xFFFF) continue;
            checkFunction(bus, device, i, vendorId);
        }
    }
}

static void checkBus(uint8_t bus) {
    for (uint8_t i = 0; i < 32; i++) {
        checkDevice(bus, i);
    }
}

void Pci::scanForDevices() {
    uint8_t headerType = readConfig(0, 0, 0, offsetof(PciHeader, headerType));
    if (headerType & PCI_HEADER_MULTIFUNCTION) {
        for (uint8_t i = 0; i < 8; i++) {
            uint16_t vendorId = readConfig(0, 0, i,
                    offsetof(PciHeader, vendorId));
            if (vendorId == 0xFFFF) continue;
            checkBus(i);
        }
    } else {
        checkBus(0);
    }
}
