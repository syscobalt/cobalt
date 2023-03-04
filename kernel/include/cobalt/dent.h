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
 *  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Dennix operating
 *  system, which can be found at https://github.com/dennis95/dennix. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#ifndef _COBALT_DENT_H
#define _COBALT_DENT_H

#include <cobalt/types.h>

struct posix_dent {
    __ino_t d_ino;
    __reclen_t d_reclen;
    unsigned char d_type;
    __extension__ char d_name[];
};

#define _IFTODT(mode) (((mode) & 0170000) >> 12)
#define _DTTOIF(type) (((type) & 017) << 12)

#define DT_FIFO 01
#define DT_CHR 02
#define DT_DIR 04
#define DT_BLK 06
#define DT_REG 010
#define DT_LNK 012
#define DT_SOCK 014
#define DT_UNKNOWN 0

#define DT_MQ 020
#define DT_SEM 020
#define DT_SHM 020

#define DT_FORCE_TYPE (1 << 0)

#define _DT_FLAGS DT_FORCE_TYPE

#endif
