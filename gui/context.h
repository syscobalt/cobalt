#pragma once

typedef struct {
    int display_fd;
    int display_height;
    int display_width;
    uint32_t framebuffer;
} __context;