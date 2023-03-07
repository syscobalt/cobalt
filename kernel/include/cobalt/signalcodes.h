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
 *  system, which can be found at https://github.com/dennis95/cobalt. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#ifndef _COBALT_SIGNALCODES_H
#define _COBALT_SIGNALCODES_H

#define SI_USER 1
#define SI_QUEUE 2
#define SI_TIMER 3
#define SI_ASYNCIO 4
#define SI_MESGQ 5
#define SI_KERNEL 6

#define ILL_ILLOPC 7
#define ILL_ILLOPN 8
#define ILL_ILLADR 9
#define ILL_ILLTRP 10
#define ILL_PRVOPC 11
#define ILL_PRVREG 12
#define ILL_COPROC 13
#define ILL_BADSTK 14

#define FPE_INTDIV 7
#define FPE_INTOVF 8
#define FPE_FLTDIV 9
#define FPE_FLTOVF 10
#define FPE_FLTUND 11
#define FPE_FLTRES 12
#define FPE_FLTINV 13
#define FPE_FLTSUB 14

#define SEGV_MAPERR 7
#define SEGV_ACCERR 8

#define BUS_ADRALN 7
#define BUS_ADRERR 8
#define BUS_OBJERR 9

#define TRAP_BRKPT 7
#define TRAP_TRACE 8

#define CLD_EXITED 7
#define CLD_KILLED 8
#define CLD_DUMPED 9
#define CLD_TRAPPED 10
#define CLD_STOPPED 11
#define CLD_CONTINUED 12

#endif
