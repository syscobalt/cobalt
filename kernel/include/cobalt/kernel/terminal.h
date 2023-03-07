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

#ifndef KERNEL_TERMINAL_H
#define KERNEL_TERMINAL_H

#include <cobalt/termios.h>
#include <cobalt/winsize.h>
#include <cobalt/kernel/keyboard.h>
#include <cobalt/kernel/vnode.h>

#define TERMINAL_BUFFER_SIZE 4096

class Terminal : public Vnode {
public:
    Terminal(dev_t dev);
    int devctl(int command, void* restrict data, size_t size,
            int* restrict info) override;
    void exitSession();
    void hangup();
    int isatty() override;
    short poll() override;
    ssize_t read(void* buffer, size_t size, int flags) override;
    void setWinsize(const struct winsize* ws);
    int tcgetattr(struct termios* result) override;
    int tcsetattr(int flags, const struct termios* termios) override;
    ssize_t write(const void* buffer, size_t size, int flags) override;
protected:
    bool canWriteBuffer();
    size_t dataAvailable();
    void endLine();
    virtual bool getTtyPath(char* buffer, size_t size) = 0;
    void handleCharacter(char c);
    virtual void output(const char* buffer, size_t size) = 0;
    void writeBuffer(char c);
private:
    bool backspace();
    bool hasIncompleteLine();
    void raiseSignal(int signal);
    char readBuffer();
    void resetBuffer();
protected:
    struct termios termio;
    bool hungup;
    pid_t sid;
private:
    pid_t foregroundGroup;
    unsigned int numEof;
    kthread_cond_t readCond;
    kthread_cond_t writeCond;
    char circularBuffer[TERMINAL_BUFFER_SIZE];
    size_t readIndex;
    size_t lineIndex;
    size_t writeIndex;
    struct winsize winsize;
};

#endif
