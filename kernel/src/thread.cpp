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

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <cobalt/kernel/process.h>
#include <cobalt/kernel/registers.h>
#include <cobalt/kernel/worker.h>

Thread* Thread::_current;
Thread* Thread::idleThread;
static Thread* firstThread;

__fpu_t initFpu;

static int bootErrno;
extern "C" { int* __errno_location = &bootErrno; }

Thread::Thread(Process* process) {
    contextChanged = false;
    forceKill = false;
    interruptContext = nullptr;
    kernelStack = 0;
    next = nullptr;
    pendingSignals = nullptr;
    prev = nullptr;
    this->process = process;
    returnSignalMask = 0;
    signalMask = 0;
    signalMutex = KTHREAD_MUTEX_INITIALIZER;
    signalCond = KTHREAD_COND_INITIALIZER;
    tid = -1;
    tlsBase = 0;
}

Thread::~Thread() {
    kernelSpace->unmapMemory(kernelStack, PAGESIZE);
}

void Thread::initializeIdleThread() {
    Process* idleProcess = xnew Process();
    idleProcess->addressSpace = kernelSpace;
    Process::addProcess(idleProcess);
    assert(idleProcess->pid == 0);
    idleThread = xnew Thread(idleProcess);
    idleThread->tid = idleProcess->threads.add(idleThread);
    assert(idleThread->tid == 0);
    _current = idleThread;
}

void Thread::addThread(Thread* thread) {
    Interrupts::disable();
    thread->next = firstThread;
    if (firstThread) {
        firstThread->prev = thread;
    }
    firstThread = thread;
    Interrupts::enable();
}

void Thread::removeThread(Thread* thread) {
    if (thread->prev) {
        thread->prev->next = thread->next;
    } else {
        firstThread = thread->next;
    }

    if (thread->next) {
        thread->next->prev = thread->prev;
    }
}

InterruptContext* Thread::schedule(InterruptContext* context) {
    if (likely(!_current->contextChanged)) {
        _current->interruptContext = context;
        Registers::saveFpu(&_current->fpuEnv);
        _current->tlsBase = getTlsBase();
    } else {
        _current->contextChanged = false;
    }

    if (_current->next) {
        _current = _current->next;
    } else {
        if (firstThread) {
            _current = firstThread;
        } else {
            _current = idleThread;
        }
    }

    setKernelStack(_current->kernelStack + PAGESIZE);
    Registers::restoreFpu(&_current->fpuEnv);
    setTlsBase(_current->tlsBase);
    __errno_location = &_current->errorNumber;

    _current->process->addressSpace->activate();
    _current->checkSigalarm(true);
    _current->updatePendingSignals();
    return _current->interruptContext;
}

static void deleteThread(void* thread) {
    delete (Thread*) thread;
}

NORETURN void Thread::terminate(bool alsoTerminateProcess) {
    assert(this == Thread::current());

    kthread_mutex_lock(&process->threadsMutex);
    process->threads[tid] = nullptr;
    kthread_mutex_unlock(&process->threadsMutex);

    WorkerJob job;
    job.func = deleteThread;
    job.context = this;

    Interrupts::disable();
    Thread::removeThread(this);
    WorkerThread::addJob(&job);
    if (alsoTerminateProcess) {
        WorkerThread::addJob(&process->terminationJob);
    }
    Interrupts::enable();

    sched_yield();
    __builtin_unreachable();
}

static void deallocateStack(void* address) {
    kernelSpace->unmapMemory((vaddr_t) address, PAGESIZE);
}

void Thread::updateContext(vaddr_t newKernelStack, InterruptContext* newContext,
            const __fpu_t* newFpuEnv) {
    Interrupts::disable();
    if (this == _current) {
        contextChanged = true;
    }

    vaddr_t oldKernelStack = kernelStack;
    kernelStack = newKernelStack;
    interruptContext = newContext;
    memcpy(fpuEnv, newFpuEnv, sizeof(__fpu_t));

    if (this == _current) {
        WorkerJob job;
        if (oldKernelStack) {
            job.func = deallocateStack;
            job.context = (void*) oldKernelStack;
            WorkerThread::addJob(&job);
        }

        sched_yield();
        __builtin_unreachable();
    } else if (oldKernelStack) {
        kernelSpace->unmapMemory(oldKernelStack, PAGESIZE);
    }

    Interrupts::enable();
}
