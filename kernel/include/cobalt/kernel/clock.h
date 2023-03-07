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

#ifndef KERNEL_CLOCK_H
#define KERNEL_CLOCK_H

#include <time.h>

class Clock {
public:
    Clock();
    void add(const Clock* clock);
    int getTime(struct timespec* result);
    int nanosleep(int flags, const struct timespec* requested,
            struct timespec* remaining);
    int setTime(struct timespec* newValue);
    void tick(unsigned long nanoseconds);
public:
    static Clock* get(clockid_t clockid);
    static void onTick(bool user, unsigned long nanoseconds);
private:
    struct timespec value;
};

struct timespec timespecPlus(struct timespec ts1, struct timespec ts2);
bool timespecLess(struct timespec ts1, struct timespec ts2);

#endif
