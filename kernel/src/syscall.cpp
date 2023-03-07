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

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <cobalt/fchownat.h>
#include <cobalt/fcntl.h>
#include <cobalt/wait.h>
#include <cobalt/kernel/addressspace.h>
#include <cobalt/kernel/clock.h>
#include <cobalt/kernel/ext234.h>
#include <cobalt/kernel/log.h>
#include <cobalt/kernel/pipe.h>
#include <cobalt/kernel/process.h>
#include <cobalt/kernel/signal.h>
#include <cobalt/kernel/streamsocket.h>
#include <cobalt/kernel/syscall.h>

static const void* syscallList[NUM_SYSCALLS] = {
    /*[SYSCALL_EXIT_THREAD] =*/ (void*) Syscall::exit_thread,
    /*[SYSCALL_WRITE] =*/ (void*) Syscall::write,
    /*[SYSCALL_READ] =*/ (void*) Syscall::read,
    /*[SYSCALL_MMAP] =*/ (void*) Syscall::mmap,
    /*[SYSCALL_MUNMAP] =*/ (void*) Syscall::munmap,
    /*[SYSCALL_OPENAT] =*/ (void*) Syscall::openat,
    /*[SYSCALL_CLOSE] =*/ (void*) Syscall::close,
    /*[SYSCALL_REGFORK] =*/ (void*) Syscall::regfork,
    /*[SYSCALL_EXECVE] =*/ (void*) Syscall::execve,
    /*[SYSCALL_WAITPID] =*/ (void*) Syscall::waitpid,
    /*[SYSCALL_FSTATAT] =*/ (void*) Syscall::fstatat,
    /*[SYSCALL_GETDENTS] =*/ (void*) Syscall::getdents,
    /*[SYSCALL_CLOCK_NANOSLEEP] =*/ (void*) Syscall::clock_nanosleep,
    /*[SYSCALL_TCGETATTR] =*/ (void*) Syscall::tcgetattr,
    /*[SYSCALL_TCSETATTR] =*/ (void*) Syscall::tcsetattr,
    /*[SYSCALL_FCHDIRAT] =*/ (void*) Syscall::fchdirat,
    /*[SYSCALL_CONFSTR] =*/ (void*) Syscall::confstr,
    /*[SYSCALL_FSTAT] =*/ (void*) Syscall::fstat,
    /*[SYSCALL_MKDIRAT] =*/ (void*) Syscall::mkdirat,
    /*[SYSCALL_UNLINKAT] =*/ (void*) Syscall::unlinkat,
    /*[SYSCALL_RENAMEAT] =*/ (void*) Syscall::renameat,
    /*[SYSCALL_LINKAT] =*/ (void*) Syscall::linkat,
    /*[SYSCALL_SYMLINKAT] =*/ (void*) Syscall::symlinkat,
    /*[SYSCALL_GETPID] =*/ (void*) Syscall::getpid,
    /*[SYSCALL_KILL] =*/ (void*) Syscall::kill,
    /*[SYSCALL_SIGACTION] =*/ (void*) Syscall::sigaction,
    /*[SYSCALL_ABORT] =*/ (void*) Syscall::abort,
    /*[SYSCALL_CLOCK_GETTIME] =*/ (void*) Syscall::clock_gettime,
    /*[SYSCALL_DUP3] =*/ (void*) Syscall::dup3,
    /*[SYSCALL_ISATTY] =*/ (void*) Syscall::isatty,
    /*[SYSCALL_PIPE2] =*/ (void*) Syscall::pipe2,
    /*[SYSCALL_LSEEK] =*/ (void*) Syscall::lseek,
    /*[SYSCALL_UMASK] =*/ (void*) Syscall::umask,
    /*[SYSCALL_FCHMODAT] =*/ (void*) Syscall::fchmodat,
    /*[SYSCALL_FCNTL] =*/ (void*) Syscall::fcntl,
    /*[SYSCALL_UTIMENSAT] =*/ (void*) Syscall::utimensat,
    /*[SYSCALL_DEVCTL] =*/ (void*) Syscall::devctl,
    /*[SYSCALL_GETPGID] =*/ (void*) Syscall::getpgid,
    /*[SYSCALL_SETPGID] =*/ (void*) Syscall::setpgid,
    /*[SYSCALL_READLINKAT] =*/ (void*) Syscall::readlinkat,
    /*[SYSCALL_FTRUNCATE] =*/ (void*) Syscall::ftruncate,
    /*[SYSCALL_SIGPROCMASK] =*/ (void*) Syscall::sigprocmask,
    /*[SYSCALL_ALARM] =*/ (void*) Syscall::alarm,
    /*[SYSCALL_FCHMOD] =*/ (void*) Syscall::fchmod,
    /*[SYSCALL_FUTIMENS] =*/ (void*) Syscall::futimens,
    /*[SYSCALL_GETRUSAGENS] =*/ (void*) Syscall::getrusagens,
    /*[SYSCALL_GETENTROPY] =*/ (void*) Syscall::getentropy,
    /*[SYSCALL_FCHDIR] =*/ (void*) Syscall::fchdir,
    /*[SYSCALL_FCHOWNAT] =*/ (void*) Syscall::fchownat,
    /*[SYSCALL_MEMINFO] =*/ (void*) Syscall::meminfo,
    /*[SYSCALL_SIGTIMEDWAIT] =*/ (void*) Syscall::sigtimedwait,
    /*[SYSCALL_PPOLL] =*/ (void*) Syscall::ppoll,
    /*[SYSCALL_SOCKET] =*/ (void*) Syscall::socket,
    /*[SYSCALL_BIND] =*/ (void*) Syscall::bind,
    /*[SYSCALL_LISTEN] =*/ (void*) Syscall::listen,
    /*[SYSCALL_CONNECT] =*/ (void*) Syscall::connect,
    /*[SYSCALL_ACCEPT4] =*/ (void*) Syscall::accept4,
    /*[SYSCALL_MOUNT] =*/ (void*) Syscall::mount,
    /*[SYSCALL_UNMOUNT] =*/ (void*) Syscall::unmount,
    /*[SYSCALL_FPATHCONF] =*/ (void*) Syscall::fpathconf,
    /*[SYSCALL_FSSYNC] =*/ (void*) Syscall::fssync,
    /*[SYSCALL_FCHOWN] =*/ (void*) Syscall::fchown,
    /*[SYSCALL_SETSID] =*/ (void*) Syscall::setsid,
    /*[SYSCALL_GETPPID] =*/ (void*) Syscall::getppid,
};

static Reference<FileDescription> getRootFd(int fd, const char* path) {
    if (path[0] == '/') {
        AutoLock lock(&Process::current()->fdMutex);
        return Process::current()->rootFd;
    } else if (fd == AT_FDCWD) {
        AutoLock lock(&Process::current()->fdMutex);
        return Process::current()->cwdFd;
    } else {
        return Process::current()->getFd(fd);
    }
}

static Reference<Vnode> resolvePathExceptLastComponent(int fd, const char* path,
        const char** lastComponent) {
    Reference<FileDescription> descr = getRootFd(fd, path);
    if (!descr) return nullptr;
    return resolvePathExceptLastComponent(descr->vnode, path, lastComponent);
}

extern "C" const void* getSyscallHandler(unsigned interruptNumber) {
    if (interruptNumber >= NUM_SYSCALLS) {
        return (void*) Syscall::badSyscall;
    } else {
        return syscallList[interruptNumber];
    }
}

NORETURN void Syscall::abort() {
    siginfo_t siginfo = {};
    siginfo.si_signo = SIGABRT;
    Process::current()->terminateBySignal(siginfo);

    sched_yield();
    __builtin_unreachable();
}

int Syscall::accept4(int fd, struct sockaddr* address, socklen_t* length,
        int flags) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;

    descr = descr->accept4(address, length, flags);
    if (!descr) return -1;

    int fdFlags = 0;
    if (flags & SOCK_CLOEXEC) fdFlags |= FD_CLOEXEC;
    if (flags & SOCK_CLOFORK) fdFlags |= FD_CLOFORK;
    return Process::current()->addFileDescriptor(descr, fdFlags);
}

unsigned int Syscall::alarm(unsigned int seconds) {
    return Process::current()->alarm(seconds);
}

int Syscall::bind(int fd, const struct sockaddr* address, socklen_t length) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->bind(address, length);
}

int Syscall::clock_gettime(clockid_t clockid, struct timespec* result) {
    Clock* clock = Clock::get(clockid);
    if (!clock) return -1;

    return clock->getTime(result);
}

int Syscall::clock_nanosleep(clockid_t clockid, int flags,
        const struct timespec* requested, struct timespec* remaining) {
    if (clockid == CLOCK_PROCESS_CPUTIME_ID) return errno = EINVAL;

    if (clockid == CLOCK_REALTIME && !(flags & TIMER_ABSTIME)) {
        clockid = CLOCK_MONOTONIC;
    }

    Clock* clock = Clock::get(clockid);
    if (!clock) return errno;

    return clock->nanosleep(flags, requested, remaining);
}

int Syscall::close(int fd) {
    return Process::current()->close(fd);
}

int Syscall::connect(int fd, const struct sockaddr* address, socklen_t length) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->connect(address, length);
}

int Syscall::devctl(int fd, int command, void* restrict data, size_t size,
        int* restrict info) {
    int dummy;
    if (!info) {
        // Set info so that drivers can assign info unconditionally.
        info = &dummy;
    }

    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) {
        *info = -1;
        return errno;
    }

    return descr->vnode->devctl(command, data, size, info);
}

int Syscall::dup3(int fd1, int fd2, int flags) {
    return Process::current()->dup3(fd1, fd2, flags);
}

int Syscall::execve(const char* path, char* const argv[], char* const envp[]) {
    Reference<FileDescription> descr = getRootFd(AT_FDCWD, path);
    Reference<Vnode> vnode = resolvePath(descr->vnode, path);
    descr = nullptr;

    if (!vnode || Process::current()->execute(vnode, argv, envp) == -1) {
        return -1;
    }

    sched_yield();
    __builtin_unreachable();
}

NORETURN void Syscall::exit_thread(const struct exit_thread* data) {
    struct exit_thread copy = *data;
    if (copy.unmapAddress) {
        Syscall::munmap(copy.unmapAddress, copy.unmapSize);
    }
    Process::current()->exitThread(&copy);
}

int Syscall::fchdir(int fd) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    if (!S_ISDIR(descr->vnode->stat().st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    Reference<FileDescription> newCwd = new FileDescription(descr->vnode,
            O_SEARCH);
    if (!newCwd) return -1;
    AutoLock lock(&Process::current()->fdMutex);
    Process::current()->cwdFd = newCwd;
    return 0;
}

int Syscall::fchdirat(int fd, const char* path) {
    Reference<FileDescription> descr = getRootFd(fd, path);
    if (!descr) return -1;
    Reference<Vnode> vnode = resolvePath(descr->vnode, path);
    if (!vnode) return -1;
    if (!S_ISDIR(vnode->stat().st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    Reference<FileDescription> newCwd = new FileDescription(vnode, O_SEARCH);
    if (!newCwd) return -1;
    AutoLock lock(&Process::current()->fdMutex);
    Process::current()->cwdFd = newCwd;
    return 0;
}

int Syscall::fchmod(int fd, mode_t mode) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->vnode->chmod(mode);
}

int Syscall::fchmodat(int fd, const char* path, mode_t mode, int flags) {
    bool followFinalSymlink = !(flags & AT_SYMLINK_NOFOLLOW);
    Reference<FileDescription> descr = getRootFd(fd, path);
    if (!descr) return -1;
    Reference<Vnode> vnode = resolvePath(descr->vnode, path,
            followFinalSymlink);
    if (!vnode) return -1;

    return vnode->chmod(mode);
}

int Syscall::fchown(int fd, uid_t uid, gid_t gid) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->vnode->chown(uid, gid);
}

static int fchownatImpl(int fd, const char* path, uid_t uid, gid_t gid,
        int flags) {
    bool followFinalSymlink = !(flags & AT_SYMLINK_NOFOLLOW);
    Reference<FileDescription> descr = getRootFd(fd, path);
    if (!descr) return -1;
    Reference<Vnode> vnode = resolvePath(descr->vnode, path,
            followFinalSymlink);
    if (!vnode) return -1;

    return vnode->chown(uid, gid);
}

int Syscall::fchownat(struct fchownatParams* params) {
    return fchownatImpl(params->fd, params->path, params->uid, params->gid,
            params->flags);
}

int Syscall::fcntl(int fd, int cmd, int param) {
    return Process::current()->fcntl(fd, cmd, param);
}

long Syscall::fpathconf(int fd, int name) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->vnode->pathconf(name);
}

int Syscall::fssync(int fd, int flags) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->vnode->sync(flags);
}

int Syscall::fstat(int fd, struct stat* result) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->vnode->stat(result);
}

int Syscall::fstatat(int fd, const char* restrict path,
        struct stat* restrict result, int flags) {
    bool followFinalSymlink = !(flags & AT_SYMLINK_NOFOLLOW);
    Reference<FileDescription> descr = getRootFd(fd, path);
    if (!descr) return -1;
    Reference<Vnode> vnode = resolvePath(descr->vnode, path,
            followFinalSymlink);
    if (!vnode) return -1;

    return vnode->stat(result);
}

int Syscall::ftruncate(int fd, off_t length) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->vnode->ftruncate(length);
}

int Syscall::futimens(int fd, const struct timespec ts[2]) {
    static struct timespec nullTs[2] = {{ 0, UTIME_NOW }, { 0, UTIME_NOW }};
    if (!ts) {
        ts = nullTs;
    }

    if (((ts[0].tv_nsec < 0 || ts[0].tv_nsec >= 1000000000) &&
            ts[0].tv_nsec != UTIME_NOW && ts[0].tv_nsec != UTIME_OMIT) ||
            ((ts[1].tv_nsec < 0 || ts[1].tv_nsec >= 1000000000) &&
            ts[1].tv_nsec != UTIME_NOW && ts[1].tv_nsec != UTIME_OMIT)) {
        errno = EINVAL;
        return -1;
    }

    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->vnode->utimens(ts[0], ts[1]);
}

ssize_t Syscall::getdents(int fd, void* buffer, size_t size, int flags) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->getdents(buffer, size, flags);
}

int Syscall::getentropy(void* buffer, size_t size) {
    if (size > GETENTROPY_MAX) {
        errno = EINVAL;
        return -1;
    }
    arc4random_buf(buffer, size);
    return 0;
}

pid_t Syscall::getpid() {
    return Process::current()->pid;
}

pid_t Syscall::getppid() {
    return Process::current()->getParentPid();
}

pid_t Syscall::getpgid(pid_t pid) {
    if (pid == 0) {
        return Process::current()->pgid;
    }
    Process* process = Process::get(pid);
    if (!process) return -1;
    return process->pgid;
}

int Syscall::getrusagens(int who, struct rusagens* usage) {
    if (who == RUSAGE_SELF) {
        Process::current()->systemCpuClock.getTime(&usage->ru_stime);
        Process::current()->userCpuClock.getTime(&usage->ru_utime);
    } else if (who == RUSAGE_CHILDREN) {
        Process::current()->childrenSystemCpuClock.getTime(&usage->ru_stime);
        Process::current()->childrenUserCpuClock.getTime(&usage->ru_utime);
    } else {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int Syscall::isatty(int fd) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return 0;
    return descr->vnode->isatty();
}

int Syscall::linkat(int oldFd, const char* oldPath, int newFd,
        const char* newPath, int flags) {
    bool followFinalSymlink = flags & AT_SYMLINK_FOLLOW;
    Reference<FileDescription> descr = getRootFd(oldFd, oldPath);
    if (!descr) return -1;
    Reference<Vnode> vnode = resolvePath(descr->vnode, oldPath,
            followFinalSymlink);
    if (!vnode) return -1;

    if (S_ISDIR(vnode->stat().st_mode)) {
        errno = EPERM;
        return -1;
    }

    const char* name;
    Reference<Vnode> directory = resolvePathExceptLastComponent(newFd,
            newPath, &name);
    if (!directory) return -1;
    return directory->link(name, vnode);
}

int Syscall::listen(int fd, int backlog) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->vnode->listen(backlog);
}

off_t Syscall::lseek(int fd, off_t offset, int whence) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->lseek(offset, whence);
}

int Syscall::mkdirat(int fd, const char* path, mode_t mode) {
    const char* name;
    Reference<Vnode> vnode = resolvePathExceptLastComponent(fd, path, &name);
    if (!vnode) return -1;
    if (!*name) {
        errno = EEXIST;
        return -1;
    }

    return vnode->mkdir(name, mode & ~Process::current()->umask());
}

static void* mmapImplementation(void* /*addr*/, size_t size,
        int protection, int flags, int /*fd*/, off_t /*offset*/) {
    if (size == 0 || !(flags & MAP_PRIVATE)) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    if (flags & MAP_ANONYMOUS) {
        AddressSpace* addressSpace = Process::current()->addressSpace;
        return (void*) addressSpace->mapMemory(ALIGNUP(size, PAGESIZE),
                protection & _PROT_FLAGS);
    }

    // TODO: Implement other flags than MAP_ANONYMOUS
    errno = ENOTSUP;
    return MAP_FAILED;
}

void* Syscall::mmap(__mmapRequest* request) {
    return mmapImplementation(request->_addr, request->_size,
            request->_protection, request->_flags, request->_fd,
            request->_offset);
}

int Syscall::mount(const char* filename, const char* mountPath,
        const char* filesystem, int flags) {
    Reference<Vnode> file = resolvePath(getRootFd(AT_FDCWD, filename)->vnode,
            filename);
    if (!file) return -1;

    const char* lastComponent;
    Reference<Vnode> mountpoint = resolvePathExceptLastComponent(AT_FDCWD,
            mountPath, &lastComponent);
    if (!mountpoint) return -1;
    mountpoint = mountpoint->getChildNode(lastComponent);
    if (!mountpoint) return -1;

    if (!S_ISDIR(mountpoint->stat().st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    FileSystem* fs = nullptr;
    if (strcmp(filesystem, "ext234") == 0 || strcmp(filesystem, "ext2") == 0 ||
            strcmp(filesystem, "ext3") == 0 ||
            strcmp(filesystem, "ext4") == 0) {
        fs = Ext234::initialize(file, mountpoint, mountPath, flags);
    } else {
        errno = EINVAL;
    }

    if (!fs) return -1;

    int result = mountpoint->mount(fs);
    if (result < 0) {
        delete fs;
    }
    return result;
}

int Syscall::munmap(void* addr, size_t size) {
    if (size == 0 || !PAGE_ALIGNED((vaddr_t) addr)) {
        errno = EINVAL;
        return -1;
    }

    AddressSpace* addressSpace = Process::current()->addressSpace;
    //TODO: The userspace process could unmap kernel pages!
    addressSpace->unmapMemory((vaddr_t) addr, ALIGNUP(size, 0x1000));
    return 0;
}

int Syscall::openat(int fd, const char* path, int flags, mode_t mode) {
    Reference<FileDescription> descr = getRootFd(fd, path);
    if (!descr) return -1;

    Reference<FileDescription> result = descr->openat(path, flags,
            mode & ~Process::current()->umask());
    if (!result) return -1;

    int fdFlags = 0;
    if (flags & O_CLOEXEC) fdFlags |= FD_CLOEXEC;
    if (flags & O_CLOFORK) fdFlags |= FD_CLOFORK;

    return Process::current()->addFileDescriptor(result, fdFlags);
}

int Syscall::pipe2(int fd[2], int flags) {
    Reference<Vnode> readPipe;
    Reference<Vnode> writePipe;
    if (!(new PipeVnode(readPipe, writePipe))) return -1;

    Reference<FileDescription> readDescr = new FileDescription(readPipe,
            O_RDONLY);
    if (!readDescr) return -1;
    Reference<FileDescription> writeDescr = new FileDescription(writePipe,
            O_WRONLY);
    if (!writeDescr) return -1;

    int fdFlags = 0;
    if (flags & O_CLOEXEC) fdFlags |= FD_CLOEXEC;
    if (flags & O_CLOFORK) fdFlags |= FD_CLOFORK;

    int fd0 = Process::current()->addFileDescriptor(readDescr, fdFlags);
    if (fd0 < 0) return -1;
    int fd1 = Process::current()->addFileDescriptor(writeDescr, fdFlags);
    if (fd1 < 0) {
        int oldErrno = errno;
        Process::current()->close(fd0);
        errno = oldErrno;
        return -1;
    }

    fd[0] = fd0;
    fd[1] = fd1;
    return 0;
}

int Syscall::ppoll(struct pollfd fds[], nfds_t nfds,
        const struct timespec* timeout, const sigset_t* sigmask) {
    struct timespec endTime;
    if (timeout) {
        if (timeout->tv_nsec < 0 || timeout->tv_nsec >= 1000000000L) {
            errno = EINVAL;
            return -1;
        }
        struct timespec now;
        Clock::get(CLOCK_MONOTONIC)->getTime(&now);
        endTime = timespecPlus(now, *timeout);
    }

    sigset_t oldMask;
    if (sigmask) {
        sigprocmask(SIG_SETMASK, sigmask, &oldMask);
    }

    int events = 0;
    while (true) {
        for (nfds_t i = 0; i < nfds; i++) {
            int fd = fds[i].fd;
            if (fd < 0) {
                fds[i].revents = 0;
                continue;
            }
            Reference<FileDescription> descr = Process::current()->getFd(fd);
            if (!descr) {
                fds[i].revents = POLLNVAL;
                events++;
                continue;
            }
            fds[i].revents = descr->vnode->poll() &
                    (fds[i].events | POLLERR | POLLHUP);
            if (fds[i].revents) events++;
        }

        if (events) {
            if (sigmask) {
                sigprocmask(SIG_SETMASK, &oldMask, nullptr);
            }
            return events;
        }
        if (timeout) {
            struct timespec now;
            Clock::get(CLOCK_MONOTONIC)->getTime(&now);
            if (!timespecLess(now, endTime)) {
                if (sigmask) {
                    sigprocmask(SIG_SETMASK, &oldMask, nullptr);
                }
                return 0;
            }
        }

        if (Signal::isPending()) {
            if (sigmask) {
                Thread::current()->returnSignalMask = oldMask;
            }
            errno = EINTR;
            return -1;
        }

        sched_yield();
    }
}

ssize_t Syscall::read(int fd, void* buffer, size_t size) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->read(buffer, size);
}

ssize_t Syscall::readlinkat(int fd, const char* restrict path,
        char* restrict buffer, size_t size) {
    Reference<FileDescription> descr = getRootFd(fd, path);
    if (!descr) return -1;

    Reference<Vnode> vnode = resolvePath(descr->vnode, path, false);
    if (!vnode) return -1;

    return vnode->readlink(buffer, size);
}

pid_t Syscall::regfork(int flags, regfork_t* registers) {
    if (flags == (RFPROC | RFFDG)) {
        Process* newProcess = Process::current()->regfork(flags, registers);
        if (!newProcess) return -1;

        return newProcess->pid;
    } else if (flags == (RFTHREAD | RFMEM)) {
        Thread* thread = Process::current()->newThread(flags, registers);
        if (!thread) return -1;
        return thread->tid;
    } else {
        errno = EINVAL;
        return -1;
    }
}

int Syscall::renameat(int oldFd, const char* oldPath, int newFd,
        const char* newPath) {
    const char* oldName;
    Reference<Vnode> oldDirectory = resolvePathExceptLastComponent(oldFd,
            oldPath, &oldName);
    if (!oldDirectory) return -1;

    const char* newName;
    Reference<Vnode> newDirectory = resolvePathExceptLastComponent(newFd,
            newPath, &newName);
    if (!newDirectory) return -1;

    if (strcmp(oldName, ".") == 0 || strcmp(oldName, "..") == 0 ||
            strncmp(oldName, "./", 2) == 0 || strncmp(oldName, "../", 3) == 0 ||
            strcmp(newName, ".") == 0 || strcmp(newName, "..") == 0 ||
            strncmp(newName, "./", 2) == 0 || strncmp(newName, "../", 3) == 0) {
        errno = EINVAL;
        return -1;
    }

    if (oldDirectory->stat().st_dev != newDirectory->stat().st_dev) {
        errno = EXDEV;
        return -1;
    }

    return newDirectory->rename(oldDirectory, oldName, newName);
}

int Syscall::setpgid(pid_t pid, pid_t pgid) {
    if (pgid < 0) {
        errno = EINVAL;
        return -1;
    }

    // TODO: Disallow changing the group of the child after it has called exec.

    Process* process;
    if (pid == 0) {
        process = Process::current();
    } else {
        process = Process::get(pid);
        if (!process) return -1;
        if (process != Process::current() &&
                !Process::current()->isParentOf(process)) {
            errno = ESRCH;
            return -1;
        }
        if (process->sid != Process::current()->sid) {
            errno = EPERM;
            return -1;
        }
    }

    return process->setpgid(pgid);
}

pid_t Syscall::setsid() {
    return Process::current()->setsid();
}

int Syscall::sigtimedwait(const sigset_t* set, siginfo_t* info,
        const struct timespec* timeout) {
    return Thread::current()->sigtimedwait(set, info, timeout);
}

int Syscall::socket(int domain, int type, int protocol) {
    Reference<Vnode> socket;

    if (domain == AF_UNIX) {
        if ((type & ~_SOCK_FLAGS) == SOCK_STREAM) {
            if (protocol != 0) {
                errno = EPROTONOSUPPORT;
                return -1;
            }

            socket = new StreamSocket(0666 & ~Process::current()->umask());
            if (!socket) return -1;
        } else {
            errno = ESOCKTNOSUPPORT;
            return -1;
        }
    } else {
        errno = EAFNOSUPPORT;
        return -1;
    }

    int fileFlags = O_RDWR;
    if (type & SOCK_NONBLOCK) fileFlags |= O_NONBLOCK;
    Reference<FileDescription> descr = new FileDescription(socket, fileFlags);
    if (!descr) return -1;

    int fdFlags = 0;
    if (type & SOCK_CLOEXEC) fdFlags |= FD_CLOEXEC;
    if (type & SOCK_CLOFORK) fdFlags |= FD_CLOFORK;
    return Process::current()->addFileDescriptor(descr, fdFlags);
}

int Syscall::symlinkat(const char* targetPath, int fd, const char* linkPath) {
    const char* name;
    Reference<Vnode> vnode = resolvePathExceptLastComponent(fd, linkPath,
            &name);
    if (!vnode) return -1;

    return vnode->symlink(targetPath, name);
}

int Syscall::tcgetattr(int fd, struct termios* result) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->tcgetattr(result);
}

int Syscall::tcsetattr(int fd, int flags, const struct termios* termio) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->tcsetattr(flags, termio);
}

mode_t Syscall::umask(mode_t newMask) {
    return Process::current()->umask(&newMask);
}

int Syscall::unlinkat(int fd, const char* path, int flags) {
    if (!(flags & (AT_REMOVEDIR | AT_REMOVEFILE))) {
        flags |= AT_REMOVEFILE;
    }

    const char* name;
    Reference<Vnode> vnode = resolvePathExceptLastComponent(fd, path, &name);
    if (!vnode) return -1;

    if (unlikely(!*name && vnode == Process::current()->rootFd->vnode)) {
        errno = EBUSY;
        return -1;
    }

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
            strncmp(name, "./", 2) == 0 || strncmp(name, "../", 3) == 0) {
        errno = EINVAL;
        return -1;
    }

    return vnode->unlink(name, flags);
}

int Syscall::unmount(const char* mountPath) {
    const char* lastComponent;
    Reference<Vnode> mountpoint = resolvePathExceptLastComponent(AT_FDCWD,
            mountPath, &lastComponent);
    if (!mountpoint) return -1;
    mountpoint = mountpoint->getChildNode(lastComponent);
    if (!mountpoint) return -1;

    return mountpoint->unmount();
}

int Syscall::utimensat(int fd, const char* path, const struct timespec ts[2],
        int flags) {
    static struct timespec nullTs[2] = {{ 0, UTIME_NOW }, { 0, UTIME_NOW }};
    if (!ts) {
        ts = nullTs;
    }

    if (((ts[0].tv_nsec < 0 || ts[0].tv_nsec >= 1000000000) &&
            ts[0].tv_nsec != UTIME_NOW && ts[0].tv_nsec != UTIME_OMIT) ||
            ((ts[1].tv_nsec < 0 || ts[1].tv_nsec >= 1000000000) &&
            ts[1].tv_nsec != UTIME_NOW && ts[1].tv_nsec != UTIME_OMIT)) {
        errno = EINVAL;
        return -1;
    }

    bool followFinalSymlink = !(flags & AT_SYMLINK_NOFOLLOW);
    Reference<FileDescription> descr = getRootFd(fd, path);
    if (!descr) return -1;
    Reference<Vnode> vnode = resolvePath(descr->vnode, path,
            followFinalSymlink);
    if (!vnode) return -1;

    return vnode->utimens(ts[0], ts[1]);
}

pid_t Syscall::waitpid(pid_t pid, int* status, int flags) {
    Process* process = Process::current()->waitpid(pid, flags);

    if (!process) {
        if (errno == 0) return 0;
        return -1;
    }

    int reason = (process->terminationStatus.si_code == CLD_EXITED)
            ? _WEXITED : _WSIGNALED;
    if (status) {
        *status = _WSTATUS(reason, process->terminationStatus.si_status);
    }
    pid_t result = process->pid;
    delete process;
    return result;
}

ssize_t Syscall::write(int fd, const void* buffer, size_t size) {
    Reference<FileDescription> descr = Process::current()->getFd(fd);
    if (!descr) return -1;
    return descr->write(buffer, size);
}

void Syscall::badSyscall() {
    siginfo_t siginfo = {};
    siginfo.si_signo = SIGSYS;
    siginfo.si_code = SI_KERNEL;
    Thread::current()->raiseSignal(siginfo);
}
