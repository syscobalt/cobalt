/* Copyright (c) 2023 Dennis Wölfing
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

/* libc/src/thread/pthread_detach.c
 * Detach a thread. (POSIX2008, called from C11)
 */

#define munmap __munmap
#include "thread.h"
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>

int __thread_detach(__thread_t thread) {
    char expected = JOINABLE;
    if (__atomic_compare_exchange_n(&thread->state, &expected, DETACHED,
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        return 0;
    } else if (expected == EXITED) {
        // The thread has already exited. Join it.
        munmap(thread->uthread.tlsCopy, thread->mappingSize);
        return 0;
    } else if (expected == DETACHED) {
        return EINVAL;
    } else {
        abort();
    }
}
__weak_alias(__thread_detach, pthread_detach);
