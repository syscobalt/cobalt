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

#include <sched.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/panic.h>
#include <cobalt/kernel/thread.h>
#include <cobalt/kernel/worker.h>

static WorkerJob* firstJob;
static WorkerJob* lastJob;

static NORETURN void worker(void) {
    while (true) {
        Interrupts::disable();
        WorkerJob* job = firstJob;
        firstJob = nullptr;
        Interrupts::enable();

        if (!job) {
            sched_yield();
        }

        while (job) {
            WorkerJob* next = job->next;
            job->func(job->context);
            job = next;
        }
    }
}

void WorkerThread::addJob(WorkerJob* job) {
    // This function needs to be called with interrupts disabled.

    job->next = nullptr;
    if (!firstJob) {
        firstJob = job;
        lastJob = job;
    } else {
        lastJob->next = job;
        lastJob = job;
    }
}

void WorkerThread::initialize() {
    Thread* thread = xnew Thread(Thread::idleThread->process);
    vaddr_t stack = kernelSpace->mapMemory(PAGESIZE, PROT_READ | PROT_WRITE);
    if (!stack) PANIC("Failed to allocate stack for worker thread");
    InterruptContext* context = (InterruptContext*)
            (stack + PAGESIZE - sizeof(InterruptContext));
    *context = {};

#ifdef __i386__
    context->eip = (vaddr_t) worker;
    context->cs = 0x8;
    context->eflags = 0x200;
    context->esp = stack + PAGESIZE - sizeof(void*);
    context->ss = 0x10;
#elif defined(__x86_64__)
    context->rip = (vaddr_t) worker;
    context->cs = 0x8;
    context->rflags = 0x200;
    context->rsp = stack + PAGESIZE - sizeof(void*);
    context->ss = 0x10;
#else
#  error "InterruptContext in WorkerThread is uninitialized."
#endif

    thread->updateContext(stack, context, &initFpu);
    Thread::addThread(thread);
}
