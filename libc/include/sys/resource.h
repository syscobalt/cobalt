/* Copyright (c) 2020 Dennis Wölfing
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

/* libc/include/sys/resource.h
 * Resource operations.
 */

#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <sys/cdefs.h>
#define __need_id_t
#define __need_suseconds_t
#define __need_time_t
#include <bits/types.h>
#include <bits/timeval.h>
#include <cobalt/resource.h>
#if __USE_COBALT
#  include <cobalt/rusagens.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define RLIM_INFINITY __UINTMAX_MAX__
#define RLIM_SAVED_MAX RLIM_INFINITY
#define RLIM_SAVED_CUR RLIM_INFINITY

#define RLIMIT_CORE 0
#define RLIMIT_CPU 1
#define RLIMIT_DATA 2
#define RLIMIT_FSIZE 3
#define RLIMIT_NOFILE 4
#define RLIMIT_STACK 5
#define RLIMIT_AS 6

typedef __UINTMAX_TYPE__ rlim_t;

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

struct rusage {
    struct timeval ru_utime;
    struct timeval ru_stime;
};

int getrlimit(int, struct rlimit*);
int getrusage(int, struct rusage*);
int setrlimit(int, const struct rlimit*);

#if __USE_COBALT
int getrusagens(int, struct rusagens*);
#endif

#ifdef __cplusplus
}
#endif

#endif
