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
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/stat.h>
#include <cobalt/poll.h>
#include <cobalt/kernel/pipe.h>
#include <cobalt/kernel/signal.h>
#include <cobalt/kernel/thread.h>

class PipeVnode::Endpoint : public Vnode {
public:
    Endpoint(const Reference<PipeVnode>& pipe)
            : Vnode(S_IFIFO | S_IRUSR | S_IWUSR, 0), pipe(pipe) {}
    int stat(struct stat* result) override;
protected:
    Reference<PipeVnode> pipe;
};

class PipeVnode::ReadEnd : public Endpoint {
public:
    ReadEnd(const Reference<PipeVnode>& pipe) : Endpoint(pipe) {}
    short poll() override;
    ssize_t read(void* buffer, size_t size, int flags) override;
    virtual ~ReadEnd();
};

class PipeVnode::WriteEnd : public Endpoint {
public:
    WriteEnd(const Reference<PipeVnode>& pipe) : Endpoint(pipe) {}
    short poll() override;
    ssize_t write(const void* buffer, size_t size, int flags) override;
    virtual ~WriteEnd();
};

PipeVnode::PipeVnode(Reference<Vnode>& readPipe, Reference<Vnode>& writePipe)
        : Vnode(S_IFIFO | S_IRUSR | S_IWUSR, 0),
        circularBuffer(pipeBuffer, sizeof(pipeBuffer)) {
    readEnd = new ReadEnd(this);
    if (!readEnd) FAIL_CONSTRUCTOR;
    writeEnd = new WriteEnd(this);
    if (!writeEnd) {
        delete readEnd;
        readEnd = nullptr;
        FAIL_CONSTRUCTOR;
    }

    readCond = KTHREAD_COND_INITIALIZER;
    writeCond = KTHREAD_COND_INITIALIZER;
    readPipe = readEnd;
    writePipe = writeEnd;
}

PipeVnode::~PipeVnode() {
    assert(!readEnd);
    assert(!writeEnd);
}

int PipeVnode::Endpoint::stat(struct stat* result) {
    return pipe->stat(result);
}

short PipeVnode::ReadEnd::poll() {
    return pipe->poll() & (POLLIN | POLLRDNORM | POLLHUP);
}

ssize_t PipeVnode::ReadEnd::read(void* buffer, size_t size, int flags) {
    return pipe->read(buffer, size, flags);
}

PipeVnode::ReadEnd::~ReadEnd() {
    AutoLock lock(&pipe->mutex);
    pipe->readEnd = nullptr;
    kthread_cond_broadcast(&pipe->writeCond);
}

short PipeVnode::WriteEnd::poll() {
    return pipe->poll() & (POLLOUT | POLLWRNORM | POLLHUP);
}

ssize_t PipeVnode::WriteEnd::write(const void* buffer, size_t size, int flags) {
    return pipe->write(buffer, size, flags);
}

PipeVnode::WriteEnd::~WriteEnd() {
    AutoLock lock(&pipe->mutex);
    pipe->writeEnd = nullptr;
    kthread_cond_broadcast(&pipe->readCond);
}

short PipeVnode::poll() {
    AutoLock lock(&mutex);
    short result = 0;
    if (circularBuffer.bytesAvailable()) result |= POLLIN | POLLRDNORM;
    if (readEnd && circularBuffer.spaceAvailable()) {
        result |= POLLOUT | POLLWRNORM;
    }
    if (!readEnd || !writeEnd) result |= POLLHUP;
    return result;
}

ssize_t PipeVnode::read(void* buffer, size_t size, int flags) {
    if (size == 0) return 0;
    AutoLock lock(&mutex);

    while (circularBuffer.bytesAvailable() == 0) {
        if (!writeEnd) return 0;

        if (flags & O_NONBLOCK) {
            errno = EAGAIN;
            return -1;
        }

        if (kthread_cond_sigwait(&readCond, &mutex) == EINTR) {
            errno = EINTR;
            return -1;
        }
    }

    size_t bytesRead = circularBuffer.read(buffer, size);
    kthread_cond_broadcast(&writeCond);
    updateTimestamps(true, false, false);
    return bytesRead;
}

ssize_t PipeVnode::write(const void* buffer, size_t size, int flags) {
    if (size == 0) return 0;
    AutoLock lock(&mutex);

    if (size <= PIPE_BUF) {
        while (circularBuffer.spaceAvailable() < size && readEnd) {
            if (flags & O_NONBLOCK) {
                errno = EAGAIN;
                return -1;
            }

            if (kthread_cond_sigwait(&writeCond, &mutex) == EINTR) {
                errno = EINTR;
                return -1;
            }
        }
    }

    const char* buf = (const char*) buffer;
    size_t written = 0;

    while (written < size) {
        while (circularBuffer.spaceAvailable() == 0 && readEnd) {
            if (flags & O_NONBLOCK) {
                if (written) {
                    updateTimestamps(false, true, true);
                    return written;
                }
                errno = EAGAIN;
                return -1;
            }

            if (kthread_cond_sigwait(&writeCond, &mutex) == EINTR) {
                if (written) {
                    updateTimestamps(false, true, true);
                    return written;
                }
                errno = EINTR;
                return -1;
            }
        }

        if (!readEnd) {
            siginfo_t siginfo = {};
            siginfo.si_signo = SIGPIPE;
            siginfo.si_code = SI_KERNEL;
            Thread::current()->raiseSignal(siginfo);
            errno = EPIPE;
            return -1;
        }

        written += circularBuffer.write(buf + written, size - written);
        kthread_cond_broadcast(&readCond);
    }

    updateTimestamps(false, true, true);
    return written;
}
