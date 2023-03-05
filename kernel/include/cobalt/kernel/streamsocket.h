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

#ifndef KERNEL_STREAMSOCKET_H
#define KERNEL_STREAMSOCKET_H

#include <cobalt/un.h>
#include <cobalt/kernel/circularbuffer.h>
#include <cobalt/kernel/socket.h>

class StreamSocket : public Socket, public ConstructorMayFail {
private:
    struct ConnectionMutex : public ReferenceCounted {
        kthread_mutex_t mutex = KTHREAD_MUTEX_INITIALIZER;
    };
public:
    StreamSocket(mode_t mode);
    StreamSocket(mode_t mode, const Reference<StreamSocket>& peer,
            const Reference<ConnectionMutex>& connection);
    ~StreamSocket();
    Reference<Vnode> accept(struct sockaddr* address, socklen_t* length,
            int fileFlags) override;
    int bind(const struct sockaddr* address, socklen_t length, int flags)
            override;
    int connect(const struct sockaddr* address, socklen_t length, int flags)
            override;
    int listen(int backlog) override;
    short poll() override;
    ssize_t read(void* buffer, size_t size, int flags) override;
    ssize_t write(const void* buffer, size_t size, int flags) override;
private:
    bool addConnection(const Reference<StreamSocket>& socket);
private:
    kthread_mutex_t socketMutex;
    kthread_cond_t acceptCond;
    kthread_cond_t connectCond;
    struct sockaddr_un boundAddress;
    bool isConnected;
    bool isConnecting;
    bool isListening;
    Reference<StreamSocket> firstConnection;
    Reference<StreamSocket> lastConnection;
    Reference<StreamSocket> nextConnection;
private:
    Reference<ConnectionMutex> connectionMutex;
    kthread_cond_t receiveCond;
    kthread_cond_t sendCond;
    StreamSocket* peer;
    char* receiveBuffer;
    CircularBuffer circularBuffer;
};

#endif
