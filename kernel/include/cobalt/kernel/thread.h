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

#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H

#include <signal.h>
#include <cobalt/kernel/clock.h>
#include <cobalt/kernel/interrupts.h>
#include <cobalt/kernel/kernel.h>
#include <cobalt/kernel/kthread.h>

class Process;

struct PendingSignal {
    siginfo_t siginfo;
    PendingSignal* next;
};

class Thread {
public:
    Thread(Process* process);
    ~Thread();
    InterruptContext* handleSignal(InterruptContext* context);
    void raiseSignal(siginfo_t siginfo);
    int sigtimedwait(const sigset_t* set, siginfo_t* info,
            const struct timespec* timeout);
    NORETURN void terminate(bool alsoTerminateProcess);
    void updateContext(vaddr_t newKernelStack, InterruptContext* newContext,
            const __fpu_t* newFpuEnv);
    void updatePendingSignals();
private:
    void checkSigalarm(bool scheduling);
    void raiseSignalUnlocked(siginfo_t siginfo);
public:
    Clock cpuClock;
    bool forceKill;
    __fpu_t fpuEnv;
    Process* process;
    sigset_t returnSignalMask;
    sigset_t signalMask;
    pid_t tid;
    uintptr_t tlsBase;
private:
    bool contextChanged;
    int errorNumber;
    InterruptContext* interruptContext;
    vaddr_t kernelStack;
    Thread* next;
    PendingSignal* pendingSignals;
    Thread* prev;
    kthread_mutex_t signalMutex;
    kthread_cond_t signalCond;
public:
    static void addThread(Thread* thread);
    static Thread* current() { return _current; }
    static Thread* idleThread;
    static void initializeIdleThread();
    static void removeThread(Thread* thread);
    static InterruptContext* schedule(InterruptContext* context);
private:
    static Thread* _current;
};

void setKernelStack(uintptr_t stack);
extern "C" {
extern __fpu_t initFpu;
}

#endif
