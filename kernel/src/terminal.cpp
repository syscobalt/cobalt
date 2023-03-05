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

#include <errno.h>
#include <signal.h>
#include <cobalt/devctls.h>
#include <cobalt/poll.h>
#include <cobalt/kernel/process.h>
#include <cobalt/kernel/signal.h>
#include <cobalt/kernel/terminal.h>

#define CTRL(c) ((c) & 0x1F)
#define UNCTRL(c) (((c) + 64) & 0x7F)

Terminal::Terminal(dev_t dev) : Vnode(S_IFCHR | 0666, dev) {
    termio.c_iflag = 0;
    termio.c_oflag = 0;
    termio.c_cflag = CREAD | CS8;
    termio.c_lflag = ECHO | ECHOE | ECHOK | ICANON | ISIG;
    termio.c_ispeed = B38400;
    termio.c_ospeed = B38400;

    termio.c_cc[VEOF] = CTRL('D');
    termio.c_cc[VEOL] = 0;
    termio.c_cc[VERASE] = '\b';
    termio.c_cc[VINTR] = CTRL('C');
    termio.c_cc[VKILL] = CTRL('U');
    termio.c_cc[VMIN] = 1;
    termio.c_cc[VQUIT] = CTRL('\\');
    termio.c_cc[VSTART] = CTRL('Q');
    termio.c_cc[VSTOP] = CTRL('S');
    termio.c_cc[VSUSP] = CTRL('Z');
    termio.c_cc[VTIME] = 0;

    foregroundGroup = -1;
    sid = -1;
    numEof = 0;
    readCond = KTHREAD_COND_INITIALIZER;
    writeCond = KTHREAD_COND_INITIALIZER;
    readIndex = 0;
    lineIndex = 0;
    writeIndex = 0;
    winsize.ws_row = 80;
    winsize.ws_col = 25;
    hungup = false;
}

static bool isSpecialChar(unsigned char c) {
    return (c < 0x20 || c == 0x7F) && c != '\t' && c != '\n' && c != '\r';
}

void Terminal::handleCharacter(char c) {
    if (termio.c_lflag & ICANON && c == termio.c_cc[VEOF]) {
        if (hasIncompleteLine()) {
            endLine();
        } else {
            numEof++;
            kthread_cond_broadcast(&readCond);
        }
    } else if (termio.c_lflag & ICANON && c == termio.c_cc[VERASE]) {
        if (backspace() && (termio.c_lflag & ECHOE)) {
            output("\b \b", 3);
        }
    } else if (termio.c_lflag & ISIG && c == termio.c_cc[VINTR]) {
        if (!(termio.c_lflag & NOFLSH)) {
            endLine();
        }
        raiseSignal(SIGINT);
    } else if (termio.c_lflag & ICANON && c == termio.c_cc[VKILL]) {
        while (backspace()) {
            if (termio.c_lflag & ECHOK) {
                output("\b \b", 3);
            }
        }
    } else if (termio.c_lflag & ISIG && c == termio.c_cc[VQUIT]) {
        raiseSignal(SIGQUIT);
    } else if (/* IXON */ false && c == termio.c_cc[VSTART]) {

    } else if (/* IXON */ false && c == termio.c_cc[VSTOP]) {

    } else if (termio.c_lflag & ISIG && c == termio.c_cc[VSUSP]) {

    } else {
        if (termio.c_lflag & ECHO || (termio.c_lflag & ECHONL && c == '\n')) {
            if (isSpecialChar(c)) {
                char seq[] = { '^', (char) UNCTRL(c) };
                output(seq, 2);
            } else {
                output(&c, 1);
            }
        }
        writeBuffer(c);
        if (!(termio.c_lflag & ICANON) || c == '\n' || c == termio.c_cc[VEOL]) {
            endLine();
        }
    }
}

void Terminal::hangup() {
    AutoLock lock(&mutex);

    if (sid != -1 && !(termio.c_cflag & CLOCAL)) {
        Process* controllingProcess = Process::get(sid);
        if (controllingProcess) {
            siginfo_t siginfo = {};
            siginfo.si_signo = SIGHUP;
            siginfo.si_code = SI_KERNEL;
            controllingProcess->raiseSignal(siginfo);
        }
    }

    hungup = true;
    kthread_cond_broadcast(&readCond);
}

int Terminal::devctl(int command, void* restrict data, size_t size,
        int* restrict info) {
    AutoLock lock(&mutex);

    switch (command) {
    case TIOCGPGRP: {
        if (size != 0 && size != sizeof(pid_t)) {
            *info = -1;
            return EINVAL;
        }

        kthread_mutex_lock(&Process::current()->jobControlMutex);
        if (Process::current()->controllingTerminal != this) {
            kthread_mutex_unlock(&Process::current()->jobControlMutex);
            *info = -1;
            return ENOTTY;
        }
        kthread_mutex_unlock(&Process::current()->jobControlMutex);

        pid_t* pgid = (pid_t*) data;

        if (foregroundGroup >= 0) {
            *pgid = foregroundGroup;
        } else {
            *pgid = INT_MAX;
        }
        *info = 0;
        return 0;
    } break;
    case TIOCGWINSZ: {
        if (size != 0 && size != sizeof(struct winsize)) {
            *info = -1;
            return EINVAL;
        }

        struct winsize* ws = (struct winsize*) data;
        *ws = winsize;
        *info = 0;
        return 0;
    } break;
    case TIOCSPGRP: {
        if (size != 0 && size != sizeof(pid_t)) {
            *info = -1;
            return EINVAL;
        }

        kthread_mutex_lock(&Process::current()->jobControlMutex);
        if (Process::current()->controllingTerminal != this) {
            kthread_mutex_unlock(&Process::current()->jobControlMutex);
            *info = -1;
            return ENOTTY;
        }
        kthread_mutex_unlock(&Process::current()->jobControlMutex);

        const pid_t* pgid = (const pid_t*) data;

        if (*pgid < 0) {
            *info = -1;
            return EINVAL;
        }

        if (!Process::getGroup(*pgid)) {
            *info = -1;
            return EPERM;
        }

        // TODO: The terminal should lose its foreground ground when the group
        // dies.
        foregroundGroup = *pgid;

        *info = 0;
        return 0;
    } break;
    case TCFLSH: {
        if (size != 0 && size != sizeof(int)) {
            *info = -1;
            return EINVAL;
        }

        const int* selector = (const int*) data;
        switch (*selector) {
        case TCIFLUSH:
        case TCIOFLUSH:
            resetBuffer();
            break;
        case TCOFLUSH:
            // Output is always transmitted immediately.
            break;
        default:
            *info = -1;
            return EINVAL;
        }
        *info = 0;
        return 0;
    } break;
    case TIOCGPATH: {
        if (size == 0) size = TTY_NAME_MAX + 1;
        if (getTtyPath((char*) data, size)) {
            *info = 0;
            return 0;
        } else {
            *info = -1;
            return ERANGE;
        }
    } break;
    case TIOCSCTTY: {
        if (data || size) {
            *info = -1;
            return EINVAL;
        }

        if (sid != -1) {
            *info = -1;
            return EPERM;
        }

        Process* process = Process::current();
        kthread_mutex_lock(&process->jobControlMutex);
        if (process->sid != process->pid) {
            kthread_mutex_unlock(&process->jobControlMutex);
            *info = -1;
            return EPERM;
        }

        process->controllingTerminal = this;
        sid = process->sid;
        foregroundGroup = process->pgid;
        kthread_mutex_unlock(&process->jobControlMutex);
        *info = 0;
        return 0;
    } break;
    case TIOCSWINSZ: {
        if (size != 0 && size != sizeof(struct winsize)) {
            *info = -1;
            return EINVAL;
        }

        const struct winsize* ws = (const struct winsize*) data;
        setWinsize(ws);
        *info = 0;
        return 0;
    } break;
    default:
        *info = -1;
        return EINVAL;
    }
}

void Terminal::exitSession() {
    sid = -1;
    foregroundGroup = -1;
}

int Terminal::isatty() {
    return 1;
}

short Terminal::poll() {
    AutoLock lock(&mutex);
    short result = 0;
    if (dataAvailable()) result |= POLLIN | POLLRDNORM;
    if (hungup) {
        result |= POLLHUP;
    } else {
        result |= POLLOUT | POLLWRNORM;
    }
    return result;
}

void Terminal::raiseSignal(int signal) {
    siginfo_t siginfo = {};
    siginfo.si_signo = signal;
    siginfo.si_code = SI_KERNEL;

    if (foregroundGroup > 0) {
        Process* group = Process::getGroup(foregroundGroup);
        if (group) {
            group->raiseSignalForGroup(siginfo);
        }
    }
}

ssize_t Terminal::read(void* buffer, size_t size, int flags) {
    if (size == 0) return 0;
    AutoLock lock(&mutex);
    char* buf = (char*) buffer;
    size_t readSize = 0;

    size_t min = termio.c_lflag & ICANON ? 1 : termio.c_cc[VMIN];
    while (readSize < size) {
        while (dataAvailable() < min && !numEof && !hungup) {
            if (readSize) {
                updateTimestamps(true, false, false);
                return readSize;
            }

            if (flags & O_NONBLOCK) {
                errno = EAGAIN;
                return -1;
            }

            if (kthread_cond_sigwait(&readCond, &mutex) == EINTR) {
                if (readSize) {
                    updateTimestamps(true, false, false);
                    return readSize;
                }
                errno = EINTR;
                return -1;
            }
        }
        min = 1;

        if (numEof || hungup) {
            if (readSize) {
                updateTimestamps(true, false, false);
                return readSize;
            }
            if (numEof) numEof--;
            return 0;
        } else if (dataAvailable() == 0) {
            return 0;
        }

        char c = readBuffer();
        buf[readSize] = c;
        readSize++;
        if ((termio.c_lflag & ICANON) && c == '\n') break;
    }

    updateTimestamps(true, false, false);
    return (ssize_t) readSize;
}

void Terminal::setWinsize(const struct winsize* ws) {
    if (winsize.ws_col == ws->ws_col && winsize.ws_row == ws->ws_row) return;
    winsize = *ws;
    raiseSignal(SIGWINCH);
}

int Terminal::tcgetattr(struct termios* result) {
    *result = termio;
    return 0;
}

int Terminal::tcsetattr(int flags, const struct termios* termio) {
    AutoLock lock(&mutex);

    if (flags == TCSANOW || flags == TCSADRAIN) {
        // TCSANOW and TCSADRAIN are identical because output is always
        // transmitted immediately.
        this->termio = *termio;
        if (!(termio->c_lflag & ICANON)) {
            endLine();
        }
    } else if (flags == TCSAFLUSH) {
        this->termio = *termio;
        resetBuffer();
        numEof = 0;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

ssize_t Terminal::write(const void* buffer, size_t size, int /*flags*/) {
    AutoLock lock(&mutex);

    if (hungup) {
        errno = EIO;
        return -1;
    }

    if (size == 0) return 0;
    const char* buf = (const char*) buffer;
    output(buf, size);

    updateTimestamps(false, true, true);
    return (ssize_t) size;
}

size_t Terminal::dataAvailable() {
    return lineIndex >= readIndex ?
            lineIndex - readIndex : readIndex - lineIndex;
}

bool Terminal::backspace() {
    if (lineIndex == writeIndex) return false;
    bool continuationByte;
    do {
        continuationByte = (circularBuffer[writeIndex - 1] & 0xC0) == 0x80;
        if (likely(writeIndex != 0)) {
            writeIndex = (writeIndex - 1) % TERMINAL_BUFFER_SIZE;
        } else {
            writeIndex = TERMINAL_BUFFER_SIZE - 1;
        }
    } while (continuationByte && lineIndex != writeIndex);

    kthread_cond_broadcast(&writeCond);
    return true;
}

bool Terminal::canWriteBuffer() {
    return (writeIndex + 1) % TERMINAL_BUFFER_SIZE != readIndex;
}

void Terminal::endLine() {
    lineIndex = writeIndex;
    kthread_cond_broadcast(&readCond);
}

bool Terminal::hasIncompleteLine() {
    return lineIndex != writeIndex;
}

char Terminal::readBuffer() {
    char result = circularBuffer[readIndex];
    readIndex = (readIndex + 1) % TERMINAL_BUFFER_SIZE;
    kthread_cond_broadcast(&writeCond);
    return result;
}

void Terminal::resetBuffer() {
    readIndex = 0;
    lineIndex = 0;
    writeIndex = 0;
    kthread_cond_broadcast(&writeCond);
}

void Terminal::writeBuffer(char c) {
    while (!canWriteBuffer()) {
        kthread_cond_sigwait(&writeCond, &mutex);
    }

    circularBuffer[writeIndex] = c;
    writeIndex = (writeIndex + 1) % TERMINAL_BUFFER_SIZE;
}
