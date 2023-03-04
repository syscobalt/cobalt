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

#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include <sys/types.h>
#include <cobalt/exit.h>
#include <cobalt/fork.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/dynarray.h>
#include <cobalt/kernel/filedescription.h>
#include <cobalt/kernel/terminal.h>
#include <cobalt/kernel/thread.h>
#include <cobalt/kernel/worker.h>

struct FdTableEntry {
    Reference<FileDescription> descr;
    int flags;

    operator bool() { return descr; }
};

class Process {
    friend Thread;
public:
    Process();
    ~Process();
    int addFileDescriptor(const Reference<FileDescription>& descr, int flags);
    unsigned int alarm(unsigned int seconds);
    int close(int fd);
    int dup3(int fd1, int fd2, int flags);
    NORETURN void exitThread(const struct exit_thread* data);
    int execute(Reference<Vnode>& vnode, char* const argv[],
            char* const envp[]);
    int fcntl(int fd, int cmd, int param);
    Reference<FileDescription> getFd(int fd);
    pid_t getParentPid();
    bool isParentOf(Process* process);
    Thread* newThread(int flags, regfork_t* registers, bool start = true);
    void raiseSignal(siginfo_t siginfo);
    void raiseSignalForGroup(siginfo_t siginfo);
    Process* regfork(int flags, regfork_t* registers);
    int setpgid(pid_t pgid);
    pid_t setsid();
    void terminate();
    void terminateBySignal(siginfo_t siginfo);
    mode_t umask(const mode_t* newMask = nullptr);
    Process* waitpid(pid_t pid, int flags);
private:
    void removeFromGroup();
public:
    AddressSpace* addressSpace;
    Clock childrenSystemCpuClock;
    Clock childrenUserCpuClock;
    Clock cpuClock;
    pid_t pid;
    Clock systemCpuClock;
    siginfo_t terminationStatus;
    DynamicArray<Thread*, pid_t> threads;
    Clock userCpuClock;

    kthread_mutex_t fdMutex;
    Reference<FileDescription> cwdFd;
    Reference<FileDescription> rootFd;

    kthread_mutex_t jobControlMutex;
    Reference<Terminal> controllingTerminal;
    pid_t pgid;
    pid_t sid;

    kthread_mutex_t signalMutex;
    struct sigaction sigactions[NSIG];

    bool ownsDisplay;
private:
    struct timespec alarmTime;
    DynamicArray<FdTableEntry, int> fdTable;
    vaddr_t sigreturn;
    bool terminated;
    WorkerJob terminationJob;
    kthread_mutex_t threadsMutex;

    kthread_mutex_t childrenMutex;
    Process* firstChild;
    Process* prevChild;
    Process* nextChild;

    kthread_mutex_t fileMaskMutex;
    mode_t fileMask;

    kthread_mutex_t groupMutex;
    Process* prevInGroup;
    Process* nextInGroup;

    kthread_mutex_t parentMutex;
    Process* parent;
public:
    static bool addProcess(Process* process);
    static Process* current() { return Thread::current()->process; }
    static Process* get(pid_t pid);
    static Process* getGroup(pid_t pgid);
    static Process* initProcess;
private:
    static int copyArguments(char* const argv[], char* const envp[],
            char**& newArgv, char**& newEnvp, AddressSpace* newAddressSpace);
    static uintptr_t loadELF(const Reference<Vnode>& vnode,
            AddressSpace* newAddressSpace, vaddr_t& tlsbase,
            vaddr_t& userStack);
};

#endif
