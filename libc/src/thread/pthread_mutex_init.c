/* Copyright (c) 2022 Dennis Wölfing
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* libc/src/thread/pthread_mutex_init.c
 * Initialize a mutex. (POSIX2008)
 */

#include "thread.h"

int pthread_mutex_init(pthread_mutex_t* restrict mutex,
        const pthread_mutexattr_t* restrict attr) {
    if (attr) {
        mutex->__type = *attr;
    } else {
        mutex->__type = PTHREAD_MUTEX_DEFAULT;
    }
    mutex->__state = UNLOCKED;
    mutex->__owner = -1;
    mutex->__count = 0;
    return 0;
}
