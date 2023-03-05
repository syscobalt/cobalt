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

#ifndef KERNEL_PIPE_H
#define KERNEL_PIPE_H

#include <cobalt/kernel/circularbuffer.h>
#include <cobalt/kernel/vnode.h>

class PipeVnode : public Vnode, public ConstructorMayFail {
private:
    // The pipe needs to reference count the read and write ends separately.
    // Thus we create classes for both ends. FileDescriptions should only be
    // opened for these two ends, but not for the pipe itself.
    class Endpoint;
    class ReadEnd;
    class WriteEnd;
public:
    PipeVnode(Reference<Vnode>& readPipe, Reference<Vnode>& writePipe);
    short poll() override;
    ssize_t read(void* buffer, size_t size, int flags) override;
    ssize_t write(const void* buffer, size_t size, int flags) override;
    virtual ~PipeVnode();
private:
    Vnode* readEnd;
    Vnode* writeEnd;
    char pipeBuffer[PIPE_BUF];
    CircularBuffer circularBuffer;
    kthread_cond_t readCond;
    kthread_cond_t writeCond;
};

#endif
