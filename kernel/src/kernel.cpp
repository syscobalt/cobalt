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

#include <assert.h>
#include <string.h>
#include <cobalt/fcntl.h>
#include <cobalt/kernel/acpi.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/console.h>
#include <cobalt/kernel/devices.h>
#include <cobalt/kernel/directory.h>
#include <cobalt/kernel/file.h>
#include <cobalt/kernel/initrd.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/panic.h>
#include <cobalt/kernel/pci.h>
#include <cobalt/kernel/physicalmemory.h>
#include <cobalt/kernel/pit.h>
#include <cobalt/kernel/process.h>
#include <cobalt/kernel/ps2.h>
#include <cobalt/kernel/rtc.h>
#include <cobalt/kernel/worker.h>

#ifndef COBALT_VERSION
#  define COBALT_VERSION ""
#endif

static void startInitProcess(void* param);
static Reference<DirectoryVnode> loadInitrd(const multiboot_info* multiboot);

/*
 *  The function `kmain` is called by `start.S`.
 */
extern "C" void kmain(uint32_t /*magic*/, paddr_t multibootAddress) {
    AddressSpace::initialize();

    // Copy the multiboot structure. This cannot fail because we just freed some
    // memory.
    vaddr_t multibootMapping;
    size_t mapSize;
    const multiboot_info* multiboot = (const multiboot_info*)
            kernelSpace->mapUnaligned(multibootAddress, sizeof(multiboot_info),
            PROT_READ, multibootMapping, mapSize);
    size_t multibootInfoSize = multiboot->total_size;
    kernelSpace->unmapPhysical(multibootMapping, mapSize);

    multiboot = (const multiboot_info*)
            kernelSpace->mapUnaligned(multibootAddress, multibootInfoSize,
            PROT_READ, multibootMapping, mapSize);

    Log::earlyInitialize(multiboot);
    // Kernel panic works after this point.

    PhysicalMemory::initialize(multiboot);

    Log::initialize();
    Log::printf("Welcome to Cobalt " COBALT_VERSION "\n");
    Interrupts::initPic();
    Acpi::initialize(multiboot);

    Log::printf("Initializing PS/2 Controller...\n");
    PS2::initialize();

    Thread::initializeIdleThread();
    Log::printf("Initializing RTC and PIT...\n");
    Rtc::initialize();
    Pit::initialize();

    Log::printf("Enabling interrupts...\n");
    Interrupts::enable();

    Log::printf("Scanning for PCI devices...\n");
    Pci::scanForDevices();

    // Load the initrd.
    Log::printf("Loading Initrd...\n");
    Reference<DirectoryVnode> rootDir = loadInitrd(multiboot);
    if (!rootDir) PANIC("Could not load initrd");
    Reference<FileDescription> rootFd = xnew FileDescription(rootDir, O_SEARCH);
    Process::current()->rootFd = rootFd;

    devFS.initialize(rootDir);
    rootDir->mkdir("tmp", 0777);
    rootDir->mkdir("run", 0755);
    rootDir->mkdir("mnt", 0755);

    WorkerJob job;
    job.func = startInitProcess;
    job.context = &rootFd;
    WorkerThread::addJob(&job);
    WorkerThread::initialize();

    while (true) {
        asm volatile ("hlt");
    }
}

static void startInitProcess(void* param) {
    Reference<FileDescription> rootFd = *(Reference<FileDescription>*) param;

    Log::printf("Starting init process...\n");
    Reference<Vnode> program = resolvePath(rootFd->vnode, "/sbin/init");
    if (!program) PANIC("No init program found");

    Process* initProcess = xnew Process();
    const char* argv[] = { "init", nullptr };
    const char* envp[] = { nullptr };
    if (initProcess->execute(program, (char**) argv, (char**) envp) < 0) {
        PANIC("Failed to start init process");
    }
    program = nullptr;
    if (!Process::addProcess(initProcess)) {
        PANIC("Failed to start init process");
    }
    assert(initProcess->pid == 1);
    Process::initProcess = initProcess;

    initProcess->controllingTerminal = console;
    Reference<FileDescription> descr = xnew FileDescription(console, O_RDWR);
    initProcess->addFileDescriptor(descr, 0); // stdin
    initProcess->addFileDescriptor(descr, 0); // stdout
    initProcess->addFileDescriptor(descr, 0); // stderr

    initProcess->rootFd = rootFd;
    initProcess->cwdFd = rootFd;
    Thread::addThread(initProcess->threads[0]);
}

/*
 *  Get the bootloader to load the initial ramdisk to memory...
 */
static Reference<DirectoryVnode> loadInitrd(const multiboot_info* multiboot) {
    uintptr_t p = (uintptr_t) multiboot + 8;

    while (true) {
        const multiboot_tag* tag = (const multiboot_tag*) p;
        if (tag->type == MULTIBOOT_TAG_TYPE_MODULE) {
            const multiboot_tag_module* moduleTag =
                    (const multiboot_tag_module*) tag;
            size_t size = ALIGNUP(moduleTag->mod_end - moduleTag->mod_start,
                    PAGESIZE);
            vaddr_t initrd = kernelSpace->mapPhysical(moduleTag->mod_start,
                    size, PROT_READ);
            if (!initrd) PANIC("Failed to map initrd");
            Reference<DirectoryVnode> root = Initrd::loadInitrd(initrd);
            kernelSpace->unmapPhysical(initrd, size);

            paddr_t address = moduleTag->mod_start;
            while (address < (moduleTag->mod_end & ~PAGE_MISALIGN)) {
                PhysicalMemory::pushPageFrame(address);
                address += PAGESIZE;
            }

            if (root->childCount) return root;
        }

        if (tag->type == MULTIBOOT_TAG_TYPE_END) {
            return nullptr;
        }

        p = ALIGNUP(p + tag->size, 8);
    }
}
