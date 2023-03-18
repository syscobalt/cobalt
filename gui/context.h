#pragma once

#include <stdint.h>

typedef struct {
    int display_fd;
    int display_height;
    int display_width;
    uint32_t *framebuffer;
    uint32_t framebuffer_size;
} __context;