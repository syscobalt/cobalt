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

#ifndef KERNEL_MOUSE_H
#define KERNEL_MOUSE_H

#include <cobalt/mouse.h>
#include <cobalt/kernel/vnode.h>

class AbsoluteMouseDriver {
public:
    virtual void setAbsoluteMouse(bool enabled) = 0;
    virtual ~AbsoluteMouseDriver() {}
};
extern AbsoluteMouseDriver* absoluteMouseDriver;

class MouseDevice : public Vnode {
public:
    MouseDevice();
    void addPacket(mouse_data data);
    int devctl(int command, void* restrict data, size_t size,
            int* restrict info) override;
    short poll() override;
    ssize_t read(void* buffer, size_t size, int flags) override;
private:
    mouse_data mouseBuffer[256];
    size_t readIndex;
    size_t available;
    kthread_cond_t readCond;
};

extern Reference<MouseDevice> mouseDevice;

#endif
