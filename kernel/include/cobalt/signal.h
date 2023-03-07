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

#ifndef _COBALT_SIGNAL_H
#define _COBALT_SIGNAL_H

/* These signal numbers are commonly used by applications. POSIX specifies
   these numbers in the kill(1) utility as part of the XSI option. */
#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGABRT 6
#define SIGKILL 9
#define SIGALRM 14
#define SIGTERM 15

#define SIGBUS 4
#define SIGCHLD 5
#define SIGCONT 7
#define SIGFPE 8
#define SIGILL 10
#define SIGPIPE 11
#define SIGSEGV 12
#define SIGSTOP 13
#define SIGSYS 16
#define SIGTRAP 17
#define SIGTSTP 18
#define SIGTTIN 19
#define SIGTTOU 20
#define SIGURG 21
#define SIGUSR1 22
#define SIGUSR2 23
#define SIGWINCH 24

#define SIGRTMIN 25
#define SIGRTMAX 32

#define _NSIG 33

#define SIG_DFL ((void (*)(int)) 0)
#define SIG_IGN ((void (*)(int)) 1)
#define SIG_ERR ((void (*)(int)) -1)

#endif
