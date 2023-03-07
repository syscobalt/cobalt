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

#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cobalt/exit.h>
#include <cobalt/fork.h>
#include <cobalt/poll.h>
#include <cobalt/syscall.h>
#include <cobalt/timespec.h>
#include <cobalt/kernel/kernel.h>

struct fchownatParams;
struct meminfo;
struct __mmapRequest;
struct stat;

namespace Syscall {

NORETURN void abort();
int accept4(int fd, struct sockaddr* address, socklen_t* length, int flags);
unsigned int alarm(unsigned int seconds);
int bind(int fd, const struct sockaddr* address, socklen_t length);
int clock_gettime(clockid_t clockid, struct timespec* result);
int clock_nanosleep(clockid_t clockid, int flags,
        const struct timespec* requested, struct timespec* remaining);
int close(int fd);
size_t confstr(int name, char* buffer, size_t size);
int connect(int fd, const struct sockaddr* address, socklen_t length);
int devctl(int fd, int command, void* restrict data, size_t size,
        int* restrict info);
int dup3(int fd1, int fd2, int flags);
int execve(const char* path, char* const argv[], char* const envp[]);
NORETURN void exit_thread(const struct exit_thread* data);
int fchdir(int);
int fchdirat(int fd, const char* path);
int fchmod(int fd, mode_t mode);
int fchmodat(int fd, const char* path, mode_t mode, int flags);
int fchown(int fd, uid_t uid, gid_t gid);
int fchownat(struct fchownatParams* params);
int fcntl(int fd, int cmd, int param);
long fpathconf(int fd, int name);
int fssync(int fd, int flags);
int fstat(int fd, struct stat* result);
int fstatat(int fd, const char* restrict path, struct stat* restrict result,
        int flags);
int ftruncate(int fd, off_t length);
int futimens(int fd, const struct timespec ts[2]);
ssize_t getdents(int fd, void* buffer, size_t size, int flags);
int getentropy(void* buffer, size_t size);
pid_t getpid();
pid_t getppid();
pid_t getpgid(pid_t pid);
int getrusagens(int who, struct rusagens* usage);
int isatty(int fd);
int kill(pid_t pid, int signal);
int linkat(int oldFd, const char* oldPath, int newFd, const char* newPath,
        int flags);
int listen(int fd, int backlog);
off_t lseek(int fd, off_t offset, int whence);
void meminfo(struct meminfo*);
int mkdirat(int fd, const char* path, mode_t mode);
void* mmap(__mmapRequest* request);
int mount(const char* filename, const char* mountPath, const char* filesystem,
        int flags);
int munmap(void* addr, size_t size);
int openat(int fd, const char* path, int flags, mode_t mode);
int pipe2(int fd[2], int flags);
int ppoll(struct pollfd fds[], nfds_t nfds, const struct timespec* timeout,
        const sigset_t* sigmask);
ssize_t read(int fd, void* buffer, size_t size);
ssize_t readlinkat(int fd, const char* restrict path, char* restrict buffer,
        size_t size);
int renameat(int oldFd, const char* oldPath, int newFd, const char* newPath);
pid_t regfork(int flags, regfork_t* registers);
int setpgid(pid_t pid, pid_t pgid);
pid_t setsid();
int sigaction(int signal, const struct sigaction* restrict action,
        struct sigaction* restrict old);
int sigprocmask(int how, const sigset_t* restrict set, sigset_t* restrict old);
int sigtimedwait(const sigset_t* set, siginfo_t* info,
        const struct timespec* timeout);
int socket(int domain, int type, int protocol);
int symlinkat(const char* targetPath, int fd, const char* linkPath);
int tcgetattr(int fd, struct termios* result);
int tcsetattr(int fd, int flags, const struct termios* termio);
mode_t umask(mode_t newMask);
int unlinkat(int fd, const char* path, int flags);
int unmount(const char* mountPath);
int utimensat(int fd, const char* path, const struct timespec ts[2], int flags);
pid_t waitpid(pid_t pid, int* status, int flags);
ssize_t write(int fd, const void* buffer, size_t size);

void badSyscall();

}

#endif
