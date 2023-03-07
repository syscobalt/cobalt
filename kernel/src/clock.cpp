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

#include <errno.h>
#include <sched.h>
#include <cobalt/kernel/clock.h>
#include <cobalt/kernel/process.h>
#include <cobalt/kernel/signal.h>

static Clock monotonicClock;
static Clock realtimeClock;

struct timespec timespecPlus(struct timespec ts1, struct timespec ts2) {
    struct timespec result;
    result.tv_sec = ts1.tv_sec + ts2.tv_sec;
    result.tv_nsec = ts1.tv_nsec + ts2.tv_nsec;
    if (result.tv_nsec >= 1000000000L) {
        result.tv_sec++;
        result.tv_nsec -= 1000000000L;
    }

    return result;
}

static struct timespec timespecMinus(struct timespec ts1, struct timespec ts2) {
    struct timespec result;
    result.tv_sec = ts1.tv_sec - ts2.tv_sec;
    result.tv_nsec = ts1.tv_nsec - ts2.tv_nsec;
    if (result.tv_nsec < 0) {
        result.tv_sec--;
        result.tv_nsec += 1000000000L;
    }

    return result;
}

bool timespecLess(struct timespec ts1, struct timespec ts2) {
    if (ts1.tv_sec < ts2.tv_sec) return true;
    if (ts1.tv_sec > ts2.tv_sec) return false;
    return ts1.tv_nsec < ts2.tv_nsec;
}

Clock::Clock() {
    value.tv_sec = 0;
    value.tv_nsec = 0;
}

void Clock::add(const Clock* clock) {
    value = timespecPlus(value, clock->value);
}

Clock* Clock::get(clockid_t clockid) {
    switch (clockid) {
    case CLOCK_MONOTONIC: return &monotonicClock;
    case CLOCK_REALTIME: return &realtimeClock;
    case CLOCK_PROCESS_CPUTIME_ID: return &Process::current()->cpuClock;
    case CLOCK_THREAD_CPUTIME_ID: return &Thread::current()->cpuClock;
    default:
        errno = EINVAL;
        return nullptr;
    }
}

int Clock::getTime(struct timespec* result) {
    *result = value;
    return 0;
}

int Clock::nanosleep(int flags, const struct timespec* requested,
        struct timespec* remaining) {
    if (requested->tv_nsec < 0 || requested->tv_nsec >= 1000000000L) {
        return errno = EINVAL;
    }

    struct timespec abstime;
    if (flags & TIMER_ABSTIME) {
        abstime = *requested;
    } else {
        abstime = timespecPlus(value, *requested);
    }

    while (timespecLess(value, abstime) && !Signal::isPending()) {
        sched_yield();
    }

    struct timespec diff = timespecMinus(abstime, value);
    if (diff.tv_sec > 0 || (diff.tv_sec == 0 && diff.tv_nsec > 0)) {
        if (remaining) *remaining = diff;
        return errno = EINTR;
    }

    return 0;
}

int Clock::setTime(struct timespec* newValue) {
    value = *newValue;
    return 0;
}

void Clock::tick(unsigned long nanoseconds) {
    value.tv_nsec += nanoseconds;

    while (value.tv_nsec >= 1000000000L) {
        value.tv_sec++;
        value.tv_nsec -= 1000000000L;
    }
}

void Clock::onTick(bool user, unsigned long nanoseconds) {
    monotonicClock.tick(nanoseconds);
    realtimeClock.tick(nanoseconds);
    Process::current()->cpuClock.tick(nanoseconds);
    if (user) {
        Process::current()->userCpuClock.tick(nanoseconds);
    } else {
        Process::current()->systemCpuClock.tick(nanoseconds);
    }
    Thread::current()->cpuClock.tick(nanoseconds);
}
