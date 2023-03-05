/***
 *                                                                                      
 *      ,ad8888ba,    ,ad8888ba,    88888888ba         db         88      888888888888  
 *     d8"'    `"8b  d8"'    `"8b   88      "8b       d88b        88           88       
 *    d8'           d8'        `8b  88      ,8P      d8'`8b       88           88       
 *    88            88          88  88aaaaaa8P'     d8'  `8b      88           88       
 *    88            88          88  88""""""8b,    d8YaaaaY8b     88           88       
 *    Y8,           Y8,        ,8P  88      `8b   d8""""""""8b    88           88       
 *     Y8a.    .a8P  Y8a.    .a8P   88      a8P  d8'        `8b   88           88       
 *      `"Y8888Y"'    `"Y8888Y"'    88888888P"  d8'          `8b  88888888888  88       
 *  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Cobalt operating
 *  system, which can be found at https://github.com/dennis95/cobalt. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#ifndef _COBALT_STAT_H
#define _COBALT_STAT_H

#include <cobalt/timespec.h>
#include <cobalt/types.h>

/* We define these macros to their traditional values because some software
   assumes these values. */
#define S_IFIFO 010000
#define S_IFCHR 020000
#define S_IFDIR 040000
#define S_IFBLK 060000
#define S_IFREG 0100000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000
/* The following values are unused and can be used for new file types:
   030000 050000 070000 080000 090000 0130000 0150000 0160000 0170000
   The value 0110000 is reserved for contiguous files. */

#define S_IFMT 0170000

#define UTIME_NOW (-1)
#define UTIME_OMIT (-2)

struct stat {
    __dev_t st_dev;
    __ino_t st_ino;
    __mode_t st_mode;
    __nlink_t st_nlink;
    __uid_t st_uid;
    __gid_t st_gid;
    __dev_t st_rdev;
    __off_t st_size;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    __blksize_t st_blksize;
    __blkcnt_t st_blocks;
};

#endif
