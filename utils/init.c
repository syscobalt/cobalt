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
 *  system, which can be found at https://github.com/dennis95/dennix. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#include <devctl.h>
#include <err.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cobalt/display.h>

#define HOME "/home/user"

int main(int argc, char* argv[]) {
    (void) argc; (void) argv;

    if (getpid() != 1) errx(1, "PID is not 1");

    chdir(HOME);
    if (setenv("HOME", HOME, 1) < 0) err(1, "setenv");
    if (setenv("PATH", "/bin:/sbin", 1) < 0) err(1, "setenv");

    const char* term = "cobalt-16color";
    int fd = open("/dev/display", O_RDONLY);
    if (fd >= 0) {
        struct display_resolution res;
        // Check whether we are running in graphics mode. In text mode this call
        // fails with ENOTSUP.
        if (posix_devctl(fd, DISPLAY_GET_RESOLUTION, &res, sizeof(res),
                NULL) == 0) {
            term = "cobalt";
        }
        close(fd);
    }

    if (setenv("TERM", term, 1) < 0) err(1, "setenv");

    pid_t childPid = fork();
    if (childPid < 0) err(1, "fork");

    if (childPid == 0) {
        setpgid(0, 0);
        tcsetpgrp(0, getpid());

        const char* args[] = { "sh", NULL };
        execv("/bin/sh", (char**) args);
        err(1, "execv: '/bin/sh'");
    }

    while (true) {
        // Wait for any orphaned processes.
        int status;
        wait(&status);
    }
}
