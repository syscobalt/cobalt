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

#ifndef KERNEL_KTHREAD_H
#define KERNEL_KTHREAD_H

#include <cobalt/kernel/clock.h>

typedef bool kthread_mutex_t;
#define KTHREAD_MUTEX_INITIALIZER false

struct kthread_cond_waiter {
    kthread_cond_waiter* prev;
    kthread_cond_waiter* next;
    bool blocked;
};

typedef struct {
    kthread_mutex_t mutex;
    kthread_cond_waiter* first;
    kthread_cond_waiter* last;
} kthread_cond_t;
#define KTHREAD_COND_INITIALIZER { KTHREAD_MUTEX_INITIALIZER, nullptr, nullptr }

int kthread_cond_broadcast(kthread_cond_t* cond);
int kthread_cond_sigclockwait(kthread_cond_t* cond, kthread_mutex_t* mutex,
        clockid_t clock, const struct timespec* endTime);
int kthread_cond_signal(kthread_cond_t* cond);
int kthread_cond_sigwait(kthread_cond_t* cond, kthread_mutex_t* mutex);
int kthread_mutex_lock(kthread_mutex_t* mutex);
int kthread_mutex_trylock(kthread_mutex_t* mutex);
int kthread_mutex_unlock(kthread_mutex_t* mutex);

// A useful class that automatically unlocks a mutex when it goes out of scope.
class AutoLock {
public:
    AutoLock(kthread_mutex_t* mutex) {
        this->mutex = mutex;
        if (mutex) {
            kthread_mutex_lock(mutex);
        }
    }

    ~AutoLock() {
        if (mutex) {
            kthread_mutex_unlock(mutex);
        }
    }
private:
    kthread_mutex_t* mutex;
};

#endif
