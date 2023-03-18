#include <devctl.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cobalt/display.h>
#include <cobalt/mouse.h>
#include <stdio.h>

#include "context.h"
#include "color.h"

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
     *  Let's calculate the framebuffer size, and push it to `context->framebuffer_size`!
     */
    context->framebuffer_size = mode.video_width * mode.video_height * sizeof(uint32_t);

    /*
     *  The value `uint32_t` represents color. Here we are setting `framebuffer` to the width of the display multiplied by the height of the display, multiplied by the
     *  the size of `unsigned int` (aka. `uint32_t`).
     */
    context->framebuffer = malloc(context->framebuffer_size);
    
    if (!context->framebuffer) {
        printf("error: could not allocate framebuffer's size...\n");
        return -1;
    }

    /*
     *  I think we can start drawing now...
     */
    
    /*for(int x = 0; x <= 100; x++)
    {
        for(int y = 0; y <= 100; y++)
        {
            context->framebuffer[y * context->display_width + x] = 100;
        }
    }

    struct display_draw draw;
    draw.lfb = context->framebuffer;
    draw.lfb_pitch = context->display_width * sizeof(uint32_t);
    draw.lfb_x = 0;
    draw.lfb_y = 0;
    draw.draw_x = 0;
    draw.draw_y = 0;
    draw.draw_width = 100;
    draw.draw_height = 100;

    posix_devctl(context->display_fd, DISPLAY_DRAW, &draw, sizeof(draw), NULL);*/

    for (int i = 0; i < context->display_width * context->display_height; i++) {
        //printf("debug: i should be drawing...\n");
        context->framebuffer[i] = RGB(255, 255, 255);
    }

    printf("success: everything went right!\n");
    return 0;
}