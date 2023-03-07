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

#ifndef KERNEL_DYNARRAY_H
#define KERNEL_DYNARRAY_H

#include <assert.h>
#include <stdlib.h>
#include <cobalt/kernel/kernel.h>

template <typename T, typename TSize = size_t>
class DynamicArray {
public:
    DynamicArray() {
        allocatedSize = 0;
        buffer = nullptr;
    }

    ~DynamicArray() {
        for (TSize i = 0; i < allocatedSize; i++) {
            buffer[i].~T();
        }
        free(buffer);
    }

    TSize add(const T& obj) {
        return addAt(0, obj);
    }

    TSize addAt(TSize index, const T& obj) {
        TSize i;
        for (i = index; i < allocatedSize; i++) {
            if (!buffer[i]) {
                buffer[i] = obj;
                return i;
            }
        }

        TSize newSize;
        if (__builtin_add_overflow(i, 1, &newSize)) {
            return (TSize) -1;
        }
        if (!resize(newSize)) return (TSize) -1;
        buffer[i] = obj;
        return i;
    }

    void clear() {
        for (TSize i = 0; i < allocatedSize; i++) {
            buffer[i].~T();
        }
        free(buffer);
        allocatedSize = 0;
        buffer = nullptr;
    }

    TSize insert(TSize index, const T& obj) {
        assert(index >= 0);
        if (index >= allocatedSize) {
            TSize newSize;
            if (__builtin_add_overflow(index, 1, &newSize)) {
                return (TSize) -1;
            }
            if (!resize(newSize)) return (TSize) -1;
        }

        buffer[index] = obj;
        return index;
    }

    TSize next(TSize index) {
        for (TSize i = index + 1; i < allocatedSize; i++) {
            if (buffer[i]) return i;
        }
        return (TSize) -1;
    }

    void remove(TSize index) {
        buffer[index] = T();
    }

    bool resize(TSize size) {
        assert(size > allocatedSize);
        T* newBuffer = (T*) reallocarray(buffer, (size_t) size, sizeof(T));
        if (!newBuffer) return false;
        buffer = newBuffer;
        for (TSize i = allocatedSize; i < size; i++) {
            new (&buffer[i]) T();
        }
        allocatedSize = size;
        return true;
    }

    T& operator[](TSize index) {
        assert(index >= 0 && index < allocatedSize);
        return buffer[index];
    }

public:
    TSize allocatedSize;
private:
    T* buffer;
};

#endif
