#include <stdio.h>
#include <stdlib.h>

int main() {
    int framebuffer_fd = open("/dev/fb0", O_RDWR);
    if (framebuffer_fd < 0) {
        printf("error: unknown error...\n");
        return -1;
    }
    struct fb_info_t fb_info;
    if (ioctl(framebuffer_fd, FBIOGET_INFO, &fb_info) < 0) {
        printf("error: there seems to be an issue with `ioctl`...\n");
        close(framebuffer_fd);
        return -1;
    }
    void* framebuffer = mmap(NULL, fb_info.pitch * fb_info.height, PROT_READ | PROT_WRITE, MAP_SHARED, framebuffer_fd, 0);
    close(framebuffer_fd);
    return 0;
}