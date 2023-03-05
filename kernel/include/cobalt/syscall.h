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

#ifndef _COBALT_SYSCALL_H
#define _COBALT_SYSCALL_H

#define SYSCALL_EXIT_THREAD 0
#define SYSCALL_WRITE 1
#define SYSCALL_READ 2
#define SYSCALL_MMAP 3
#define SYSCALL_MUNMAP 4
#define SYSCALL_OPENAT 5
#define SYSCALL_CLOSE 6
#define SYSCALL_REGFORK 7
#define SYSCALL_EXECVE 8
#define SYSCALL_WAITPID 9
#define SYSCALL_FSTATAT 10
#define SYSCALL_GETDENTS 11
#define SYSCALL_CLOCK_NANOSLEEP 12
#define SYSCALL_TCGETATTR 13
#define SYSCALL_TCSETATTR 14
#define SYSCALL_FCHDIRAT 15
#define SYSCALL_CONFSTR 16
#define SYSCALL_FSTAT 17
#define SYSCALL_MKDIRAT 18
#define SYSCALL_UNLINKAT 19
#define SYSCALL_RENAMEAT 20
#define SYSCALL_LINKAT 21
#define SYSCALL_SYMLINKAT 22
#define SYSCALL_GETPID 23
#define SYSCALL_KILL 24
#define SYSCALL_SIGACTION 25
#define SYSCALL_ABORT 26
#define SYSCALL_CLOCK_GETTIME 27
#define SYSCALL_DUP3 28
#define SYSCALL_ISATTY 29
#define SYSCALL_PIPE2 30
#define SYSCALL_LSEEK 31
#define SYSCALL_UMASK 32
#define SYSCALL_FCHMODAT 33
#define SYSCALL_FCNTL 34
#define SYSCALL_UTIMENSAT 35
#define SYSCALL_DEVCTL 36
#define SYSCALL_GETPGID 37
#define SYSCALL_SETPGID 38
#define SYSCALL_READLINKAT 39
#define SYSCALL_FTRUNCATE 40
#define SYSCALL_SIGPROCMASK 41
#define SYSCALL_ALARM 42
#define SYSCALL_FCHMOD 43
#define SYSCALL_FUTIMENS 44
#define SYSCALL_GETRUSAGENS 45
#define SYSCALL_GETENTROPY 46
#define SYSCALL_FCHDIR 47
#define SYSCALL_FCHOWNAT 48
#define SYSCALL_MEMINFO 49
#define SYSCALL_SIGTIMEDWAIT 50
#define SYSCALL_PPOLL 51
#define SYSCALL_SOCKET 52
#define SYSCALL_BIND 53
#define SYSCALL_LISTEN 54
#define SYSCALL_CONNECT 55
#define SYSCALL_ACCEPT4 56
#define SYSCALL_MOUNT 57
#define SYSCALL_UNMOUNT 58
#define SYSCALL_FPATHCONF 59
#define SYSCALL_FSSYNC 60
#define SYSCALL_FCHOWN 61
#define SYSCALL_SETSID 62
#define SYSCALL_GETPPID 63

#define NUM_SYSCALLS 64

#endif
