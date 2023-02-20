/* Copyright (c) 2016, 2017 Dennis Wölfing
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

/* kernel/include/cobalt/wait.h
 * Waiting for other processes.
 */

#ifndef _COBALT_WAIT_H
#define _COBALT_WAIT_H

#define WNOHANG (1 << 0)
#define WUNTRACED (1 << 1)

#define _WEXITED 0
#define _WSIGNALED 1
#define _WSTATUS(reason, si_status) ((reason) << 24 | ((si_status) & 0xFF))

#define WEXITSTATUS(status) ((status) & 0xFF)
#define WIFEXITED(status) (((status) >> 24 & 0xFF) == _WEXITED)
#define WIFSIGNALED(status) (((status) >> 24 & 0xFF) == _WSIGNALED)
#define WTERMSIG(status) ((status) & 0xFF)

#endif
