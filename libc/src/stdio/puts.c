/* Copyright (c) 2016, 2022 Dennis Wölfing
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

/* libc/src/stdio/puts.c
 * Puts a string and a newline into stdout. (C89)
 */

#define flockfile __flockfile
#define funlockfile __funlockfile
#define putchar_unlocked __putchar_unlocked
#include <stdio.h>

static inline int puts_unlocked(const char* s) {
    while (*s) {
        if (putchar_unlocked(*s++) < 0) return EOF;
    }
    if (putchar_unlocked('\n') < 0) return EOF;
    return 1;
}

int puts(const char* s) {
    flockfile(stdout);
    int result = puts_unlocked(s);
    funlockfile(stdout);
    return result;;
}
