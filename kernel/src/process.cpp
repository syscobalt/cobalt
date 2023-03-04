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

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <sys/stat.h>
#include <cobalt/fcntl.h>
#include <cobalt/uthread.h>
#include <cobalt/wait.h>
#include <cobalt/kernel/console.h>
#include <cobalt/kernel/elf.h>
#include <cobalt/kernel/file.h>
#include <cobalt/kernel/process.h>
#include <cobalt/kernel/registers.h>
#include <cobalt/kernel/signal.h>
#include <cobalt/kernel/worker.h>

#define USER_STACK_SIZE (128 * 1024) // 128 KiB

Process* Process::initProcess;

struct ProcessTableEntry {
    Process* process;
    // This is either an actual group leader or some process of the group that
    // acts as a pseudo group leader if the group does not have a leader.
    Process* processGroup;

    operator bool() { return process || processGroup; }
};

kthread_mutex_t processesMutex = KTHREAD_MUTEX_INITIALIZER;
static DynamicArray<ProcessTableEntry, pid_t> processes;

extern "C" {
extern symbol_t beginSigreturn;
extern symbol_t endSigreturn;
}

static void terminateProcess(void* process) {
    ((Process*) process)->terminate();
}

Process::Process() {
    addressSpace = nullptr;
    pid = -1;
    terminationStatus = {};

    fdMutex = KTHREAD_MUTEX_INITIALIZER;
    cwdFd = nullptr;
    rootFd = nullptr;

    jobControlMutex = KTHREAD_MUTEX_INITIALIZER;
    controllingTerminal = nullptr;
    pgid = -1;
    sid = -1;

    signalMutex = KTHREAD_MUTEX_INITIALIZER;
    memset(sigactions, '\0', sizeof(sigactions));

    ownsDisplay = false;

    alarmTime.tv_nsec = -1;
    sigreturn = 0;
    terminated = false;
    terminationJob.func = terminateProcess;
    terminationJob.context = this;
    threadsMutex = KTHREAD_MUTEX_INITIALIZER;

    childrenMutex = KTHREAD_MUTEX_INITIALIZER;
    firstChild = nullptr;
    prevChild = nullptr;
    nextChild = nullptr;

    fileMaskMutex = KTHREAD_MUTEX_INITIALIZER;
    fileMask = S_IWGRP | S_IWOTH;

    groupMutex = KTHREAD_MUTEX_INITIALIZER;
    prevInGroup = nullptr;
    nextInGroup = nullptr;

    parentMutex = KTHREAD_MUTEX_INITIALIZER;
    parent = nullptr;
}

Process::~Process() {
    assert(terminated);
}

bool Process::addProcess(Process* process) {
    AutoLock lock(&processesMutex);
    Process* group = process->pgid == -1 ? process : nullptr;
    process->pid = processes.add({process, group});
    if (process->pgid == -1) {
        process->pgid = process->pid;
        process->sid = process->pid;
    }
    return process->pid != -1;
}

int Process::copyArguments(char* const argv[], char* const envp[],
        char**& newArgv, char**& newEnvp, AddressSpace* newAddressSpace) {
    int argc;
    int envc;
    size_t stringSizes = 0;

    for (argc = 0; argv[argc]; argc++) {
        stringSizes += strlen(argv[argc]) + 1;
    }
    for (envc = 0; envp[envc]; envc++) {
        stringSizes += strlen(envp[envc]) + 1;
    }

    stringSizes = ALIGNUP(stringSizes, alignof(char*));

    size_t size = ALIGNUP(stringSizes + (argc + envc + 2) * sizeof(char*),
            PAGESIZE);

    vaddr_t page = newAddressSpace->mapMemory(size, PROT_READ | PROT_WRITE);
    if (!page) {
        errno = ENOMEM;
        return -1;
    }
    vaddr_t pageMapped = kernelSpace->mapFromOtherAddressSpace(newAddressSpace,
            page, size, PROT_WRITE);
    if (!pageMapped) {
        errno = ENOMEM;
        return -1;
    }

    char* nextString = (char*) pageMapped;
    char** argvMapped = (char**) (pageMapped + stringSizes);
    char** envpMapped = argvMapped + argc + 1;

    for (int i = 0; i < argc; i++) {
        argvMapped[i] = (char*) ((vaddr_t) nextString - pageMapped + page);
        nextString = stpcpy(nextString, argv[i]) + 1;
    }
    for (int i = 0; i < envc; i++) {
        envpMapped[i] = (char*) ((vaddr_t) nextString - pageMapped + page);
        nextString = stpcpy(nextString, envp[i]) + 1;
    }
    argvMapped[argc] = nullptr;
    envpMapped[envc] = nullptr;

    kernelSpace->unmapPhysical(pageMapped, size);

    newArgv = (char**) (page + stringSizes);
    newEnvp = (char**) (page + stringSizes + (argc + 1) * sizeof(char*));
    return argc;
}

uintptr_t Process::loadELF(const Reference<Vnode>& vnode,
        AddressSpace* newAddressSpace, vaddr_t& tlsbase, vaddr_t& userStack) {
    ElfHeader header;
    ssize_t readSize = vnode->pread(&header, sizeof(header), 0, 0);
    if (readSize < 0) return 0;
    if (readSize != sizeof(header) ||
            memcmp(header.e_ident, "\x7F""ELF", 4) != 0) {
        errno = ENOEXEC;
        return 0;
    }

    vaddr_t tlsMaster = 0;
    size_t tlsMasterSize = 0;

    for (size_t i = 0; i < header.e_phnum; i++) {
        ProgramHeader programHeader;
        readSize = vnode->pread(&programHeader, sizeof(programHeader),
                header.e_phoff + i * header.e_phentsize, 0);
        if (readSize < 0) return 0;
        if (readSize != sizeof(programHeader)) {
            errno = ENOEXEC;
            return 0;
        }

        if (programHeader.p_type != PT_LOAD && programHeader.p_type != PT_TLS) {
            continue;
        }

        if (programHeader.p_type == PT_TLS && tlsMaster != 0) {
            errno = ENOEXEC;
            return 0;
        }

        int protection = 0;
        if (programHeader.p_flags & PF_X) protection |= PROT_EXEC;
        if (programHeader.p_flags & PF_W) protection |= PROT_WRITE;
        if (programHeader.p_flags & PF_R) protection |= PROT_READ;

        vaddr_t loadAddressAligned = 0;
        ptrdiff_t offset = 0;
        size_t size = 0;
        if (programHeader.p_type == PT_LOAD) {
            loadAddressAligned = programHeader.p_vaddr & ~PAGE_MISALIGN;
            offset = programHeader.p_vaddr - loadAddressAligned;

            size = ALIGNUP(programHeader.p_memsz + offset, PAGESIZE);

            if (!newAddressSpace->mapMemory(loadAddressAligned, size,
                    protection)) {
                errno = ENOMEM;
                return 0;
            }
        } else if (programHeader.p_type == PT_TLS) {
            size = ALIGNUP(programHeader.p_memsz, PAGESIZE);
            loadAddressAligned = newAddressSpace->mapMemory(size, protection);
            tlsMaster = loadAddressAligned;
            tlsMasterSize = programHeader.p_memsz;
            if (!loadAddressAligned) {
                errno = ENOMEM;
                return 0;
            }
            offset = 0;
        }

        vaddr_t dest = kernelSpace->mapFromOtherAddressSpace(newAddressSpace,
                loadAddressAligned, size, PROT_WRITE);
        if (!dest) {
            errno = ENOMEM;
            return 0;
        }
        memset((void*) (dest + offset), 0, programHeader.p_memsz);
        readSize = vnode->pread((void*) (dest + offset), programHeader.p_filesz,
                programHeader.p_offset, 0);
        if (readSize < 0) {
            kernelSpace->unmapPhysical(dest, size);
            return 0;
        }
        if ((uint64_t) readSize != programHeader.p_filesz) {
            kernelSpace->unmapPhysical(dest, size);
            errno = ENOEXEC;
            return 0;
        }
        kernelSpace->unmapPhysical(dest, size);
    }

    size_t uthreadOffset = ALIGNUP(tlsMasterSize, alignof(struct uthread));
    size_t tlsCopySize = ALIGNUP(uthreadOffset + UTHREAD_SIZE, PAGESIZE);

    vaddr_t tlsCopy = newAddressSpace->mapMemory(tlsCopySize,
            PROT_READ | PROT_WRITE);
    if (!tlsCopy) {
        errno = ENOMEM;
        return 0;
    }

    vaddr_t src;
    if (tlsMaster) {
        src = kernelSpace->mapFromOtherAddressSpace(newAddressSpace,
            tlsMaster, ALIGNUP(tlsMasterSize, PAGESIZE), PROT_READ);
        if (!src) {
            errno = ENOMEM;
            return 0;
        }
    }

    vaddr_t dest = kernelSpace->mapFromOtherAddressSpace(newAddressSpace,
            tlsCopy, tlsCopySize, PROT_WRITE);
    if (!dest) {
        if (tlsMaster) {
            kernelSpace->unmapPhysical(src, ALIGNUP(tlsMasterSize, PAGESIZE));
        }
        errno = ENOMEM;
        return 0;
    }
    memset((void*) dest, 0, tlsCopySize);
    if (tlsMaster) {
        memcpy((void*) dest, (void*) src, tlsMasterSize);
        kernelSpace->unmapPhysical(src, ALIGNUP(tlsMasterSize, PAGESIZE));
    }

    userStack = newAddressSpace->mapMemory(USER_STACK_SIZE,
            PROT_READ | PROT_WRITE);
    if (!userStack) {
        kernelSpace->unmapPhysical(dest, tlsCopySize);
        errno = ENOMEM;
        return 0;
    }

    tlsbase = tlsCopy + uthreadOffset;
    struct uthread* uthread = (struct uthread*) (dest + uthreadOffset);
    uthread->self = (struct uthread*) tlsbase;
    uthread->tlsMaster = (void*) tlsMaster;
    uthread->tlsSize = tlsMasterSize;
    uthread->tlsCopy = (void*) tlsCopy;
    uthread->stack = (void*) userStack;
    uthread->stackSize = USER_STACK_SIZE;
    uthread->tid = 0;
    kernelSpace->unmapPhysical(dest, tlsCopySize);
    return header.e_entry;
}

int Process::addFileDescriptor(const Reference<FileDescription>& descr,
        int flags) {
    AutoLock lock(&fdMutex);
    int fd = fdTable.add({ descr, flags });

    if (fd < 0) {
        errno = EMFILE;
    }

    return fd;
}

unsigned int Process::alarm(unsigned int seconds) {
    struct timespec now;
    Clock::get(CLOCK_REALTIME)->getTime(&now);
    Interrupts::disable();

    unsigned int remaining;
    if (alarmTime.tv_nsec != -1) {
        remaining = alarmTime.tv_sec - now.tv_sec;
        if (alarmTime.tv_nsec > now.tv_nsec) remaining++;
    } else {
        remaining = 0;
    }

    if (seconds == 0) {
        alarmTime.tv_nsec = -1;
    } else {
        alarmTime.tv_sec = now.tv_sec + seconds;
        alarmTime.tv_nsec = now.tv_nsec;
    }

    Interrupts::enable();
    return remaining;
}

int Process::close(int fd) {
    AutoLock lock(&fdMutex);
    if (fd < 0 || fd >= fdTable.allocatedSize || !fdTable[fd]) {
        errno = EBADF;
        return -1;
    }

    fdTable[fd] = { nullptr, 0 };
    return 0;
}

int Process::dup3(int fd1, int fd2, int flags) {
    if (fd1 == fd2) {
        errno = EINVAL;
        return -1;
    }

    AutoLock lock(&fdMutex);
    if (fd1 < 0 || fd1 >= fdTable.allocatedSize || !fdTable[fd1]) {
        errno = EBADF;
        return -1;
    }

    int fdFlags = 0;
    if (flags & O_CLOEXEC) fdFlags |= FD_CLOEXEC;
    if (flags & O_CLOFORK) fdFlags |= FD_CLOFORK;

    return fdTable.insert(fd2, { fdTable[fd1].descr, fdFlags });
}

int Process::execute(Reference<Vnode>& vnode, char* const argv[],
        char* const envp[]) {
    mode_t mode = vnode->stat().st_mode;
    if (!S_ISREG(mode) || !(mode & 0111)) {
        errno = EACCES;
        return -1;
    }

    // Load the program
    AddressSpace* newAddressSpace = new AddressSpace();
    if (!newAddressSpace) return -1;
    vaddr_t tlsbase;
    vaddr_t userStack;
    uintptr_t entry = loadELF(vnode, newAddressSpace, tlsbase, userStack);
    if (!entry) {
        delete newAddressSpace;
        return -1;
    }
    vnode = nullptr;

    size_t sigreturnSize = (uintptr_t) &endSigreturn -
            (uintptr_t) &beginSigreturn;
    assert(sigreturnSize <= PAGESIZE);
    sigreturn = newAddressSpace->mapMemory(PAGESIZE, PROT_EXEC);
    if (!sigreturn) {
        delete newAddressSpace;
        errno = ENOMEM;
        return -1;
    }

    vaddr_t sigreturnMapped = kernelSpace->mapFromOtherAddressSpace(
            newAddressSpace, sigreturn, PAGESIZE, PROT_WRITE);
    if (!sigreturnMapped) {
        delete newAddressSpace;
        errno = ENOMEM;
        return -1;
    }
    memcpy((void*) sigreturnMapped, &beginSigreturn, sigreturnSize);
    kernelSpace->unmapPhysical(sigreturnMapped, PAGESIZE);

    vaddr_t newKernelStack = kernelSpace->mapMemory(PAGESIZE,
            PROT_READ | PROT_WRITE);
    if (!newKernelStack) {
        delete newAddressSpace;
        errno = ENOMEM;
        return -1;
    }

    InterruptContext* newInterruptContext = (InterruptContext*)
            (newKernelStack + PAGESIZE - sizeof(InterruptContext));

    memset(newInterruptContext, 0, sizeof(InterruptContext));

    char** newArgv;
    char** newEnvp;
    int argc = copyArguments(argv, envp, newArgv, newEnvp, newAddressSpace);
    if (argc < 0) {
        kernelSpace->unmapMemory(newKernelStack, PAGESIZE);
        delete newAddressSpace;
        return -1;
    }

#ifdef __i386__
    // Pass argc, argv and envp to the process.
    newInterruptContext->eax = argc;
    newInterruptContext->ebx = (uint32_t) newArgv;
    newInterruptContext->ecx = (uint32_t) newEnvp;
    newInterruptContext->eip = (uint32_t) entry;
    newInterruptContext->cs = 0x1B;
    newInterruptContext->eflags = 0x200; // Interrupt enable
    newInterruptContext->esp = (uint32_t) userStack + USER_STACK_SIZE;
    newInterruptContext->ss = 0x23;
#elif defined(__x86_64__)
    newInterruptContext->rdi = argc;
    newInterruptContext->rsi = (vaddr_t) newArgv;
    newInterruptContext->rdx = (vaddr_t) newEnvp;
    newInterruptContext->rip = entry;
    newInterruptContext->cs = 0x1B;
    newInterruptContext->rflags = 0x200; // Interrupt enable
    newInterruptContext->rsp = userStack + USER_STACK_SIZE;
    newInterruptContext->ss = 0x23;
#endif

    kthread_mutex_lock(&threadsMutex);
    if (Thread::current()->forceKill) {
        // If the thread is being killed, do not proceed.
        kthread_mutex_unlock(&threadsMutex);
        kernelSpace->unmapMemory(newKernelStack, PAGESIZE);
        delete newAddressSpace;
        return -1;
    }

    for (pid_t tid = threads.next(-1); tid >= 0; tid = threads.next(tid)) {
        if (tid != Thread::current()->tid) {
            threads[tid]->forceKill = true;
        }
    }

    while (threads.next(-1) != -1 &&
            (threads.next(-1) != Thread::current()->tid ||
            threads.next(Thread::current()->tid) != -1)) {
        kthread_mutex_unlock(&threadsMutex);
        sched_yield();
        kthread_mutex_lock(&threadsMutex);
    }
    kthread_mutex_unlock(&threadsMutex);

    // Close all file descriptors marked with FD_CLOEXEC.
    for (int i = fdTable.next(-1); i >= 0; i = fdTable.next(i)) {
        if (fdTable[i].flags & FD_CLOEXEC) {
            fdTable[i] = { nullptr, 0 };
        }
    }

    AddressSpace* oldAddressSpace = addressSpace;
    addressSpace = newAddressSpace;
    if (this == current()) {
        addressSpace->activate();
    }
    delete oldAddressSpace;

    for (int i = 0; i < NSIG; i++) {
        sigactions[i].sa_mask = 0;
        sigactions[i].sa_flags = 0;
        if (sigactions[i].sa_handler != SIG_IGN) {
            sigactions[i].sa_handler = SIG_DFL;
        }
    }

    if (threads.next(-1) == -1) {
        Thread* thread = new Thread(this);
        if (!thread) {
            kernelSpace->unmapMemory(newKernelStack, PAGESIZE);
            delete newAddressSpace;
            return -1;
        }
        thread->tid = threads.add(thread);
        if (thread->tid == -1) {
            kernelSpace->unmapMemory(newKernelStack, PAGESIZE);
            delete thread;
            delete newAddressSpace;
            return -1;
        }
        thread->tlsBase = tlsbase;
        thread->updateContext(newKernelStack, newInterruptContext, &initFpu);
    } else {
        Thread* thread = threads[threads.next(-1)];
        if (thread->tid != 0) {
            threads[thread->tid] = nullptr;
            threads[0] = thread;
            thread->tid = 0;
        }
        thread->tlsBase = tlsbase;
        thread->updateContext(newKernelStack, newInterruptContext, &initFpu);
    }

    return 0;
}

void Process::exitThread(const struct exit_thread* data) {
    kthread_mutex_lock(&threadsMutex);
    if (Thread::current()->forceKill) {
        kthread_mutex_unlock(&threadsMutex);
        Thread::current()->terminate(false);
    }

    if (data->flags & EXIT_PROCESS) {
        for (pid_t tid = threads.next(-1); tid >= 0; tid = threads.next(tid)) {
            if (tid != Thread::current()->tid) {
                threads[tid]->forceKill = true;
            }
        }

        while (threads.next(-1) != Thread::current()->tid ||
                    threads.next(Thread::current()->tid) != -1) {
            kthread_mutex_unlock(&threadsMutex);
            sched_yield();
            kthread_mutex_lock(&threadsMutex);
        }
    }

    bool terminating = false;
    if (threads.next(-1) == Thread::current()->tid &&
            threads.next(Thread::current()->tid) == -1) {
        terminating = true;
        terminationStatus.si_signo = SIGCHLD;
        terminationStatus.si_code = CLD_EXITED;
        terminationStatus.si_pid = pid;
        terminationStatus.si_status = data->status;
    }
    kthread_mutex_unlock(&threadsMutex);

    Thread::current()->terminate(terminating);
}

int Process::fcntl(int fd, int cmd, int param) {
    AutoLock lock(&fdMutex);
    if (fd < 0 || fd >= fdTable.allocatedSize || !fdTable[fd]) {
        errno = EBADF;
        return -1;
    }

    FdTableEntry& entry = fdTable[fd];

    switch (cmd) {
        case F_DUPFD:
            return fdTable.addAt(param, { entry.descr, 0 });
        case F_DUPFD_CLOEXEC:
            return fdTable.addAt(param, { entry.descr, FD_CLOEXEC });
        case F_DUPFD_CLOFORK:
            return fdTable.addAt(param, { entry.descr, FD_CLOFORK });
        case F_GETFD:
            return entry.flags;
        case F_SETFD:
            entry.flags = param;
            return 0;
        default:
            return entry.descr->fcntl(cmd, param);
    }
}

Process* Process::get(pid_t pid) {
    AutoLock lock(&processesMutex);
    if (pid >= processes.allocatedSize || !processes[pid].process) {
        errno = ESRCH;
        return nullptr;
    }
    return processes[pid].process;
}

Reference<FileDescription> Process::getFd(int fd) {
    AutoLock lock(&fdMutex);
    if (fd < 0 || fd >= fdTable.allocatedSize || !fdTable[fd]) {
        errno = EBADF;
        return nullptr;
    }

    return fdTable[fd].descr;
}

pid_t Process::getParentPid() {
    AutoLock lock(&parentMutex);
    return parent->pid;
}

Process* Process::getGroup(pid_t pgid) {
    AutoLock lock(&processesMutex);
    if (pgid >= processes.allocatedSize || !processes[pgid].processGroup) {
        errno = ESRCH;
        return nullptr;
    }
    return processes[pgid].processGroup;
}

bool Process::isParentOf(Process* process) {
    AutoLock lock(&parentMutex);
    return this == process->parent;
}

Thread* Process::newThread(int /*flags*/, regfork_t* registers,
        bool start /*= true*/) {
    AutoLock lock(&threadsMutex);

    if (Thread::current()->forceKill) {
        return nullptr;
    }

    vaddr_t kernelStack = kernelSpace->mapMemory(PAGESIZE,
            PROT_READ | PROT_WRITE);
    if (!kernelStack) {
        errno = ENOMEM;
        return nullptr;
    }

    InterruptContext* newInterruptContext = (InterruptContext*)
            (kernelStack + PAGESIZE - sizeof(InterruptContext));
    Registers::restore(newInterruptContext, registers);

    Thread* thread = new Thread(this);
    if (!thread) {
        kernelSpace->unmapMemory(kernelStack, PAGESIZE);
        return nullptr;
    }

    thread->tlsBase = registers->__tlsbase;
    pid_t tid = threads.add(thread);
    if (tid < 0) {
        delete thread;
        kernelSpace->unmapMemory(kernelStack, PAGESIZE);
        return nullptr;
    }
    thread->tid = tid;

    __fpu_t fpuEnvironment;
    Registers::saveFpu(&fpuEnvironment);
    thread->updateContext(kernelStack, newInterruptContext, &fpuEnvironment);
    thread->signalMask = Thread::current()->signalMask;

    if (start) {
        Thread::addThread(thread);
    }
    return thread;
}

Process* Process::regfork(int flags, regfork_t* registers) {
    Process* process = new Process();
    if (!process) return nullptr;
    process->parent = this;

    Thread* thread = process->newThread(flags, registers, false);
    if (!thread) {
        process->terminate();
        delete process;
        return nullptr;
    }
    thread->signalMask = Thread::current()->signalMask;

    // Fork the address space
    process->addressSpace = addressSpace->fork();
    if (!process->addressSpace) {
        process->terminate();
        delete thread;
        delete process;
        return nullptr;
    }

    // Copy the file descriptor table except for fds with FD_CLOFORK set.
    kthread_mutex_lock(&fdMutex);
    for (int i = fdTable.next(-1); i >= 0; i = fdTable.next(i)) {
        if (fdTable[i].flags & FD_CLOFORK) continue;
        if (process->fdTable.insert(i, fdTable[i]) < 0) {
            kthread_mutex_unlock(&fdMutex);
            process->terminate();
            delete thread;
            delete process;
            return nullptr;
        }
    }
    process->cwdFd = cwdFd;
    process->rootFd = rootFd;
    kthread_mutex_unlock(&fdMutex);

    kthread_mutex_lock(&jobControlMutex);
    process->controllingTerminal = controllingTerminal;
    process->pgid = pgid;
    process->sid = sid;
    kthread_mutex_unlock(&jobControlMutex);

    kthread_mutex_lock(&signalMutex);
    memcpy(process->sigactions, sigactions, sizeof(sigactions));
    kthread_mutex_unlock(&signalMutex);

    process->sigreturn = sigreturn;
    kthread_mutex_lock(&fileMaskMutex);
    process->fileMask = fileMask;
    kthread_mutex_unlock(&fileMaskMutex);

    if (!addProcess(process)) {
        process->terminate();
        delete thread;
        delete process;
        return nullptr;
    }

    kthread_mutex_lock(&childrenMutex);
    if (firstChild) {
        process->nextChild = firstChild;
        firstChild->prevChild = process;
    }
    firstChild = process;
    kthread_mutex_unlock(&childrenMutex);

    AutoLock lock(&processesMutex);
    Process* groupLeader = processes[pgid].processGroup;
    kthread_mutex_lock(&groupLeader->groupMutex);
    process->prevInGroup = this;
    process->nextInGroup = nextInGroup;
    if (nextInGroup) {
        nextInGroup->prevInGroup = process;
    }
    nextInGroup = process;
    kthread_mutex_unlock(&groupLeader->groupMutex);

    Thread::addThread(thread);
    return process;
}

void Process::removeFromGroup() {
    // processesMutex must be held when calling this function.

    Process* groupLeader = processes[pgid].processGroup;
    kthread_mutex_lock(&groupLeader->groupMutex);

    if (!prevInGroup) {
        // This is the (pseudo) group leader.
        assert(this == groupLeader);
        if (nextInGroup) {
            // That process becomes the pseudo group leader.
            groupLeader = nextInGroup;
            kthread_mutex_lock(&groupLeader->groupMutex);
            groupLeader->prevInGroup = nullptr;
            processes[pgid].processGroup = groupLeader;
            nextInGroup = nullptr;
            kthread_mutex_unlock(&groupMutex);
        } else {
            // The group ceases to exist.
            processes[pgid].processGroup = nullptr;
        }
    } else {
        prevInGroup->nextInGroup = nextInGroup;
        if (nextInGroup) {
            nextInGroup->prevInGroup = prevInGroup;
        }
        prevInGroup = nullptr;
        nextInGroup = nullptr;
    }
    kthread_mutex_unlock(&groupLeader->groupMutex);
}

int Process::setpgid(pid_t pgid) {
    AutoLock lock(&jobControlMutex);

    if (pgid == 0) {
        pgid = pid;
    }

    if (this->pgid == pgid) return 0;

    if (sid == pid) {
        errno = EPERM;
        return -1;
    }

    AutoLock lock2(&processesMutex);
    if (pgid >= processes.allocatedSize ||
            (!processes[pgid].processGroup && pgid != pid) ||
            (processes[pgid].processGroup &&
            processes[pgid].processGroup->sid != sid)) {
        errno = EPERM;
        return -1;
    }

    removeFromGroup();
    this->pgid = pgid;

    if (!processes[pgid].processGroup) {
        processes[pgid].processGroup = this;
    } else {
        Process* groupLeader = processes[pgid].processGroup;

        AutoLock lock3(&groupLeader->groupMutex);
        prevInGroup = groupLeader;
        nextInGroup = groupLeader->nextInGroup;
        groupLeader->nextInGroup = this;
        if (nextInGroup) {
            nextInGroup->prevInGroup = this;
        }
    }

    return 0;
}

pid_t Process::setsid() {
    AutoLock lock(&jobControlMutex);
    AutoLock lock2(&processesMutex);
    if (processes[pid].processGroup) {
        errno = EPERM;
        return -1;
    }

    removeFromGroup();
    pgid = pid;
    sid = pid;
    controllingTerminal = nullptr;
    processes[pid].processGroup = this;
    return pgid;
}

void Process::terminate() {
    assert(threads.next(-1) == -1);

    if (ownsDisplay) {
        console->display->releaseDisplay();
    }

    kthread_mutex_lock(&processesMutex);
    removeFromGroup();
    kthread_mutex_unlock(&processesMutex);

    rootFd = nullptr;
    cwdFd = nullptr;
    fdTable.clear();

    if (sid == pid && controllingTerminal) {
        controllingTerminal->exitSession();
    }
    controllingTerminal = nullptr;

    kthread_mutex_lock(&childrenMutex);
    if (firstChild) {
        AutoLock lock(&initProcess->childrenMutex);

        Process* child = firstChild;
        while (true) {
            // Reassign the now orphaned processes to the init process.
            kthread_mutex_lock(&child->parentMutex);
            child->parent = initProcess;
            kthread_mutex_unlock(&child->parentMutex);
            if (!child->nextChild) {
                child->nextChild = initProcess->firstChild;
                if (initProcess->firstChild) {
                    initProcess->firstChild->prevChild = child;
                }
                initProcess->firstChild = firstChild;
                break;
            }
            child = child->nextChild;
        }
    }
    kthread_mutex_unlock(&childrenMutex);

    // Send SIGCHLD to the parent.
    if (likely(terminationStatus.si_signo == SIGCHLD)) {
        AutoLock lock(&parentMutex);
        parent->raiseSignal(terminationStatus);
    }

    delete addressSpace;
    terminated = true;
}

void Process::terminateBySignal(siginfo_t siginfo) {
    assert(this == Process::current());
    kthread_mutex_lock(&threadsMutex);

    for (pid_t tid = threads.next(-1); tid >= 0; tid = threads.next(tid)) {
        if (tid != Thread::current()->tid) {
            threads[tid]->forceKill = true;
        }
    }

    while (threads.next(-1) != Thread::current()->tid ||
                threads.next(Thread::current()->tid) != -1) {
        kthread_mutex_unlock(&threadsMutex);
        sched_yield();
        kthread_mutex_lock(&threadsMutex);
    }

    terminationStatus.si_signo = SIGCHLD;
    terminationStatus.si_code = CLD_KILLED;
    terminationStatus.si_pid = pid;
    terminationStatus.si_status = siginfo.si_signo;

    kthread_mutex_unlock(&threadsMutex);
    Thread::current()->terminate(true);
}

mode_t Process::umask(const mode_t* newMask /*= nullptr*/) {
    AutoLock lock(&fileMaskMutex);

    mode_t oldMask = fileMask;
    if (newMask) {
        fileMask = *newMask & 0777;
    }
    return oldMask;
}

Process* Process::waitpid(pid_t pid, int flags) {
    Process* process;

    if (pid == -1) {
        while (true) {
            kthread_mutex_lock(&childrenMutex);
            if (!firstChild) {
                kthread_mutex_unlock(&childrenMutex);
                errno = ECHILD;
                return nullptr;
            }

            process = firstChild;
            while (process && !process->terminated) {
                process = process->nextChild;
            }
            kthread_mutex_unlock(&childrenMutex);
            if (process) break;
            if (flags & WNOHANG) {
                return nullptr;
            }

            sched_yield();

            if (Signal::isPending()) {
                errno = EINTR;
                return nullptr;
            }
        }
    } else {
        kthread_mutex_lock(&childrenMutex);
        process = firstChild;

        while (process && process->pid != pid) {
            process = process->nextChild;
        }
        kthread_mutex_unlock(&childrenMutex);

        if (!process) {
            errno = ECHILD;
            return nullptr;
        }

        while (!process->terminated) {
            if (flags & WNOHANG) {
                return nullptr;
            }

            sched_yield();
            if (Signal::isPending()) {
                errno = EINTR;
                return nullptr;
            }
        }
    }

    childrenSystemCpuClock.add(&process->systemCpuClock);
    childrenSystemCpuClock.add(&process->childrenSystemCpuClock);
    childrenUserCpuClock.add(&process->userCpuClock);
    childrenUserCpuClock.add(&process->childrenUserCpuClock);

    kthread_mutex_lock(&childrenMutex);
    if (process->nextChild) {
        process->nextChild->prevChild = process->prevChild;
    }
    if (process->prevChild) {
        process->prevChild->nextChild = process->nextChild;
    } else {
        firstChild = process->nextChild;
    }
    kthread_mutex_unlock(&childrenMutex);

    AutoLock lock(&processesMutex);
    processes[process->pid].process = nullptr;

    return process;
}
