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

#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#define ALIGNED(alignment) __attribute__((__aligned__(alignment)))
#define NORETURN __attribute__((__noreturn__))
#define PACKED __attribute__((__packed__))
#define restrict __restrict
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#define PRINTF_LIKE(format, firstArg) \
    __attribute__((__format__(__printf__, format, firstArg)))

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define ALIGNUP(val, alignment) ((((val) - 1) & ~((alignment) - 1)) + (alignment))
#define PAGE_MISALIGN (PAGESIZE - 1)
#define PAGE_ALIGNED(value) !((value) & PAGE_MISALIGN)

// Define an incomplete type for symbols so we can only take their addresses
typedef struct _incomplete_type symbol_t;

typedef uintptr_t paddr_t;
typedef uintptr_t vaddr_t;

// Placement new
inline void* operator new(size_t /*size*/, void* addr) {
    return addr;
}
inline void* operator new[](size_t /*size*/, void* addr) {
    return addr;
}
inline void operator delete(void*, void*) {};
inline void operator delete[](void*, void*) {};

class ConstructorMayFail {
public:
    bool __constructionFailed = false;

    // Disallow placement new.
    void* operator new(size_t size) { return ::operator new(size); }
};

#define FAIL_CONSTRUCTOR do { __constructionFailed = true; return; } while (0)

NORETURN void panic(const char* file, unsigned int line, const char* func,
        const char* format, ...) PRINTF_LIKE(4, 5);

class __new {
private:
    // This is just needed for the template magic below.
    struct YesType { int yes; };
    struct NoType { int no; };
    static YesType test(ConstructorMayFail*);
    static NoType test(void*);
public:
    __new() : file(nullptr) {}
    __new(const char* file, unsigned int line, const char* func) : file(file),
            line(line), func(func) {}
    const __new& operator*(const __new&) const { return *this; }

    template <typename T, typename = decltype(test((T*) nullptr).yes)>
    ALWAYS_INLINE T* operator*(T* ptr) const {
        if (unlikely(file && !ptr)) {
            panic(file, line, func, "Allocation failure");
        } else if (unlikely(file && ptr->__constructionFailed)) {
            panic(file, line, func, "Construction failure");
        } else if (unlikely(ptr && ptr->__constructionFailed)) {
            delete ptr;
            return nullptr;
        }
        return ptr;
    }

    template <typename T, typename = decltype(test((T*) nullptr).no),
            typename = T>
    ALWAYS_INLINE T* operator*(T* ptr) const {
        if (unlikely(file && !ptr)) {
            panic(file, line, func, "Allocation failure");
        }
        return ptr;
    }
private:
    const char* file;
    unsigned int line;
    const char* func;
};

// Behaves mostly like the normal new keyword but also returns nullptr if the
// constructor fails.
#define new __new() * new
// Behaves like new but causes a kernel panic on failure.
#define xnew __new(__FILE__, __LINE__, __func__) * new

#endif
