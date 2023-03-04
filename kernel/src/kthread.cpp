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

#include <errno.h>
#include <sched.h>
#include <cobalt/kernel/kthread.h>
#include <cobalt/kernel/signal.h>

int kthread_cond_broadcast(kthread_cond_t* cond) {
    kthread_mutex_lock(&cond->mutex);
    while (cond->first) {
        kthread_cond_waiter* waiter = cond->first;
        cond->first = waiter->next;
        __atomic_store_n(&waiter->blocked, false, __ATOMIC_RELEASE);
    }
    cond->last = nullptr;

    kthread_mutex_unlock(&cond->mutex);
    return 0;
}

int kthread_cond_sigclockwait(kthread_cond_t* cond, kthread_mutex_t* mutex,
        clockid_t clock, const struct timespec* endTime) {
    kthread_mutex_lock(&cond->mutex);
    kthread_mutex_unlock(mutex);

    kthread_cond_waiter waiter;
    waiter.prev = cond->last;
    waiter.next = nullptr;
    waiter.blocked = true;
    if (cond->last) {
        cond->last->next = &waiter;
    } else {
        cond->first = &waiter;
    }
    cond->last = &waiter;
    kthread_mutex_unlock(&cond->mutex);

    int result = 0;

    while (__atomic_load_n(&waiter.blocked, __ATOMIC_ACQUIRE)) {
        if (endTime) {
            struct timespec now;
            Clock::get(clock)->getTime(&now);
            if (!timespecLess(now, *endTime)) {
                result = ETIMEDOUT;
                break;
            }
        }

        if (Signal::isPending()) {
            result = EINTR;
            break;
        }
        sched_yield();
    }

    if (result) {
        kthread_mutex_lock(&cond->mutex);

        // Only remove the waiter from the list if we were not unblocked
        // concurrently. In that case the waiter was already removed.
        if (__atomic_load_n(&waiter.blocked, __ATOMIC_RELAXED)) {
            if (waiter.prev) {
                waiter.prev->next = waiter.next;
            } else {
                cond->first = waiter.next;
            }
            if (waiter.next) {
                waiter.next->prev = waiter.prev;
            } else {
                cond->last = waiter.prev;
            }
        }
        kthread_mutex_unlock(&cond->mutex);
    }

    kthread_mutex_lock(mutex);
    return result;
}

int kthread_cond_signal(kthread_cond_t* cond) {
    kthread_mutex_lock(&cond->mutex);
    if (cond->first) {
        kthread_cond_waiter* waiter = cond->first;
        cond->first = waiter->next;
        __atomic_store_n(&waiter->blocked, false, __ATOMIC_RELEASE);
    }
    if (cond->first) {
        cond->first->prev = nullptr;
    } else {
        cond->last = nullptr;
    }
    kthread_mutex_unlock(&cond->mutex);
    return 0;
}

int kthread_cond_sigwait(kthread_cond_t* cond, kthread_mutex_t* mutex) {
    return kthread_cond_sigclockwait(cond, mutex, CLOCK_MONOTONIC, nullptr);
}

int kthread_mutex_lock(kthread_mutex_t* mutex) {
    while (__atomic_test_and_set(mutex, __ATOMIC_ACQUIRE)) {
        sched_yield();
    }
    return 0;
}

int kthread_mutex_trylock(kthread_mutex_t* mutex) {
    if (__atomic_test_and_set(mutex, __ATOMIC_ACQUIRE)) {
        return EBUSY;
    }
    return 0;
}

int kthread_mutex_unlock(kthread_mutex_t* mutex) {
    __atomic_clear(mutex, __ATOMIC_RELEASE);
    return 0;
}
