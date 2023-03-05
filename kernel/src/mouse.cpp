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
#include <fcntl.h>
#include <sched.h>
#include <cobalt/poll.h>
#include <cobalt/kernel/devices.h>
#include <cobalt/kernel/mouse.h>
#include <cobalt/kernel/signal.h>

#define BUFFER_ITEMS (sizeof(mouseBuffer) / sizeof(mouse_data))

Reference<MouseDevice> mouseDevice;
AbsoluteMouseDriver* absoluteMouseDriver = nullptr;

MouseDevice::MouseDevice() : Vnode(S_IFCHR | 0666, DevFS::dev) {
    readIndex = 0;
    available = 0;
    readCond = KTHREAD_COND_INITIALIZER;
}

void MouseDevice::addPacket(mouse_data data) {
    AutoLock lock(&mutex);

    if (available == BUFFER_ITEMS) {
        // If the buffer is full then probably noone is reading, so we will just
        // discard the oldest packet.
        available--;
        readIndex = (readIndex + 1) % BUFFER_ITEMS;
    }

    size_t writeIndex = (readIndex + available) % BUFFER_ITEMS;
    mouseBuffer[writeIndex] = data;
    available++;
    kthread_cond_broadcast(&readCond);
}

int MouseDevice::devctl(int command, void* restrict data, size_t size,
            int* restrict info) {
    AutoLock lock(&mutex);

    switch (command) {
    case MOUSE_SET_ABSOLUTE: {
        if (size != 0 && size != sizeof(int)) {
            *info = -1;
            return EINVAL;
        }

        int* enabled = (int*) data;
        if (absoluteMouseDriver) {
            absoluteMouseDriver->setAbsoluteMouse(*enabled);
        } else if (*enabled) {
            *info = -1;
            return ENOTSUP;
        }

        *info = 0;
        return 0;
    } break;
    default:
        *info = -1;
        return EINVAL;
    }
}

short MouseDevice::poll() {
    AutoLock lock(&mutex);
    if (available) return POLLIN | POLLRDNORM;
    return 0;
}

ssize_t MouseDevice::read(void* buffer, size_t size, int flags) {
    AutoLock lock(&mutex);
    // We only allow reads of whole packets to prevent synchronization issues.
    size_t packets = size / sizeof(mouse_data);
    mouse_data* buf = (mouse_data*) buffer;

    for (size_t i = 0; i < packets; i++) {
        while (!available) {
            if (i > 0) return i * sizeof(mouse_data);

            if (flags & O_NONBLOCK) {
                errno = EAGAIN;
                return -1;
            }

            if (kthread_cond_sigwait(&readCond, &mutex) == EINTR) {
                errno = EINTR;
                return -1;
            }
        }

        buf[i] = mouseBuffer[readIndex];
        readIndex = (readIndex + 1) % BUFFER_ITEMS;
        available--;
    }

    return packets * sizeof(mouse_data);
}
