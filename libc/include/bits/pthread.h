/* Copyright (c) 2022, 2023 Dennis Wölfing
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

/* libc/include/bits/pthread.h
 * Pthread types.
 */

#ifndef _BITS_PTHREAD_H
#define _BITS_PTHREAD_H

#include <bits/thread.h>

typedef __thread_t pthread_t;
typedef __thread_attr_t pthread_attr_t;
typedef __cond_t pthread_cond_t;
typedef __clockid_t pthread_condattr_t;
typedef __key_t pthread_key_t;
typedef __mutex_t pthread_mutex_t;
typedef int pthread_mutexattr_t;
typedef __once_t pthread_once_t;

#endif
