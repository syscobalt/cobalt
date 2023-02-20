/* Copyright (c) 2019, 2020, 2021, 2022 Dennis Wölfing
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

/* kernel/include/cobalt/limits.h
 * Implementation limits.
 */

#ifndef _DENNIX_LIMITS_H
#define _DENNIX_LIMITS_H

#define FILESIZEBITS 64
#define _GETENTROPY_MAX 256
#define _NSIG_MAX 65
#define PAGESIZE 0x1000
#define PAGE_SIZE PAGESIZE
#define PIPE_BUF 4096
#define RTSIG_MAX 8
#define SYMLOOP_MAX 20
#define TTY_NAME_MAX 20

#endif
