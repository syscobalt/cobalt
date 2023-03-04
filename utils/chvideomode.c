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
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cobalt/display.h>

static bool parseMode(const char* str, struct video_mode* mode);

int main(int argc, char* argv[]) {
    if (argc > 2) {
        errx(1, "extra operand '%s'", argv[2]);
    }
    if (argc == 2) {
        struct video_mode mode;
        if (!parseMode(argv[1], &mode)) {
            errx(1, "invalid videomode '%s'", argv[1]);
        }
        int fd = open("/dev/display", O_RDONLY);
        if (fd < 0) err(1, "open: '/dev/display'");
        struct video_mode modeCopy = mode;
        errno = posix_devctl(fd, DISPLAY_SET_VIDEO_MODE, &mode, sizeof(mode),
                NULL);
        if (errno) err(1, "cannot set video mode '%s'", argv[1]);
        close(fd);

        if (modeCopy.video_width != mode.video_width ||
                modeCopy.video_height != mode.video_height) {
            warnx("video mode was set to %ux%ux%u", mode.video_width,
                    mode.video_height, mode.video_bpp);
        }
    } else {
        struct video_mode mode;
        int fd = open("/dev/display", O_RDONLY);
        if (fd < 0) err(1, "open: '/dev/display'");
        errno = posix_devctl(fd, DISPLAY_GET_VIDEO_MODE, &mode, sizeof(mode),
                NULL);
        if (errno) err(1, "cannot get video mode");
        close(fd);
        printf("%ux%ux%u\n", mode.video_width, mode.video_height,
                mode.video_bpp);
    }
}

static bool parseMode(const char* str, struct video_mode* mode) {
    char* end;
    unsigned long width = strtoul(str, &end, 10);
    if (width == 0 || width > UINT_MAX) return false;
    if (*end != 'x') return false;
    unsigned long height = strtoul(end + 1, &end, 10);
    if (height == 0 || height > UINT_MAX) return false;
    unsigned long bpp = 32;
    if (*end == 'x') {
        bpp = strtoul(end + 1, &end, 10);
        if (bpp > 32) return false;
    }
    if (*end != '\0') return false;

    mode->video_width = width;
    mode->video_height = height;
    mode->video_bpp = bpp;
    return true;
}
