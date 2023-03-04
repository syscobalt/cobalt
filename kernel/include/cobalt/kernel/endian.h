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

#ifndef KERNEL_ENDIAN_H
#define KERNEL_ENDIAN_H

#include <endian.h>
#include <cobalt/kernel/kernel.h>

template <typename T, int E>
class Endian {
    static_assert(E == LITTLE_ENDIAN || E == BIG_ENDIAN,
            "Unsupported endianness");
    static_assert(BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == BIG_ENDIAN,
            "Unsupported endianness");
    static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
            "Unsupported type size");
private:
    static T convert(const T& value) {
        if (E == BYTE_ORDER) {
            return value;
        }

        if (sizeof(T) == 2) {
            return __bswap16(value);
        } else if (sizeof(T) == 4) {
            return __bswap32(value);
        } else if (sizeof(T) == 8) {
            return __bswap64(value);
        }
    }
public:
    Endian() {
        value = 0;
    }

    Endian(const T& val) {
        value = convert(val);
    }

    template<typename T2, int E2>
    Endian(const Endian<T2, E2>& other) {
        T2 val = other;
        value = convert(val);
    }

    operator T() const {
        return convert(value);
    }

    Endian& operator=(const T& other) {
        value = convert(other);
        return *this;
    }

    template<typename T2, int E2>
    Endian& operator=(const Endian<T2, E2>& other) {
        T2 val = other;
        value = convert(val);
        return *this;
    }
private:
T value;
};

typedef uint8_t little_uint8_t;
typedef Endian<uint16_t, LITTLE_ENDIAN> little_uint16_t;
typedef Endian<uint32_t, LITTLE_ENDIAN> little_uint32_t;
typedef Endian<uint64_t, LITTLE_ENDIAN> little_uint64_t;

typedef uint8_t big_uint8_t;
typedef Endian<uint16_t, BIG_ENDIAN> big_uint16_t;
typedef Endian<uint32_t, BIG_ENDIAN> big_uint32_t;
typedef Endian<uint64_t, BIG_ENDIAN> big_uint64_t;

#endif
