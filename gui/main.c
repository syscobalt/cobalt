#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cobalt/display.h>

#include "context.h"

__context* context;

int main()
{
    context = calloc(1, sizeof(__context));
    if (!context)
        return -1;
    
    context->display_fd = open("/dev/display", O_RDONLY | O_CLOEXEC);
    if (context->display_fd < 0) {
        free(context);
        return -1;
    }

    if (posix_devctl(context->display_fd, DISPLAY_ACQUIRE, NULL, 0, NULL) != 0) {
        close(context->display_fd);
        free(context);
        return -1;
    }

    struct video_mode mode;
    if (posix_devctl(context->display_fd, DISPLAY_GET_VIDEO_MODE, &mode, sizeof(mode), NULL) != 0) {
        printf("error: could not get video mode...\n");
        return -1;
    }

    /*
     *  Set some values in context so that we know the characteristics of the display...
     */
    context->display_width = mode.video_width;
    context->display_height = mode.video_height;
    
    /*
     *  The value `uint32_t` represents color. Here we are setting `framebuffer` to the width of the display multiplied by the height of the display, multiplied by the
     *  the size of `unsigned int` (aka. `uint32_t`).
     */
    context->framebuffer = malloc(mode.video_width * mode.video_height * sizeof(uint32_t));
    
    if (!context->framebuffer) {
        printf("error: could not get framebuffer size...\n");
        return -1;
    }

    /*
     *  I think we can start drawing now...
     */

    printf("success: everything went right!\n");
    return 0;
}