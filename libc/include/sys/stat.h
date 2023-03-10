/* Copyright (c) 2016, 2017, 2018, 2019, 2020 Dennis Wölfing
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

/* libc/include/sys/stat.h
 * File information.
 */

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/cdefs.h>
#define __need_blkcnt_t
#define __need_blksize_t
#define __need_dev_t
#define __need_gid_t
#define __need_ino_t
#define __need_mode_t
#define __need_nlink_t
#define __need_off_t
#define __need_time_t
#define __need_uid_t
#include <bits/types.h>
#include <bits/stat.h>
#include <cobalt/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

#define st_atime st_atim.tv_sec
#define st_ctime st_ctim.tv_sec
#define st_mtime st_mtim.tv_sec

int chmod(const char*, mode_t);
int fchmod(int, mode_t);
int fchmodat(int, const char*, mode_t, int);
int fstat(int, struct stat*);
int fstatat(int, const char* __restrict, struct stat* __restrict, int);
int futimens(int, const struct timespec[2]);
int lstat(const char* __restrict, struct stat* __restrict);
int mkdir(const char*, mode_t);
int mkdirat(int, const char*, mode_t);
int stat(const char* __restrict, struct stat* __restrict);
mode_t umask(mode_t);
int utimensat(int, const char*, const struct timespec[2], int);

#ifdef __cplusplus
}
#endif

#endif
