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

#include "utils.h"
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    struct option longopts[] = {
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 1 },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "p", longopts, NULL)) != -1) {
        switch (c) {
        case 0:
            return help(argv[0], "[OPTIONS] UTILITY [ARGS...]\n"
                    "  -p                       (ignored)\n"
                    "      --help               display this help\n"
                    "      --version            display version info");
        case 1:
            return version(argv[0]);
        case 'p':
            break;
        case '?':
            return 1;
        }
    }

    if (optind >= argc) errx(1, "missing operand");

    // POSIX does not mention this but existing implementations of time do print
    // timing information when they get signalled by the terminal.
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGQUIT);
    sigset_t oldMask;
    sigprocmask(SIG_BLOCK, &sigset, &oldMask);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid_t pid = fork();
    if (pid < 0) {
        err(1, "fork");
    } else if (pid == 0) {
        sigprocmask(SIG_SETMASK, &oldMask, NULL);
        execvp(argv[optind], argv + optind);
        err(errno == ENOENT ? 127 : 126, "execvp: '%s'", argv[optind]);
    } else {
        int status;
        waitpid(pid, &status, 0);

        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        struct timespec realTime;
        realTime.tv_sec = end.tv_sec - start.tv_sec;
        realTime.tv_nsec = end.tv_nsec - start.tv_nsec;
        if (realTime.tv_nsec < 0) {
            realTime.tv_sec--;
            realTime.tv_nsec += 1000000000;
        }

        struct rusage usage;
        getrusage(RUSAGE_CHILDREN, &usage);
        fprintf(stderr, "real %jd.%06ld\nuser %jd.%06ld\nsys %jd.%06ld\n",
                (intmax_t) realTime.tv_sec, realTime.tv_nsec / 1000,
                (intmax_t) usage.ru_utime.tv_sec, (long) usage.ru_utime.tv_usec,
                (intmax_t) usage.ru_stime.tv_sec,
                (long) usage.ru_stime.tv_usec);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            sigprocmask(SIG_SETMASK, &oldMask, NULL);
            signal(WTERMSIG(status), SIG_DFL);
            raise(WTERMSIG(status));
        }
    }
}
