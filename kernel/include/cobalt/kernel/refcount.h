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

#ifndef KERNEL_REFCOUNT_H
#define KERNEL_REFCOUNT_H

#include <cobalt/kernel/kernel.h>

class ReferenceCounted {
public:
    ReferenceCounted();
    virtual ~ReferenceCounted();
    void addReference() const;
    virtual void removeReference() const;
    size_t getRefCount() const { return refcount; }
private:
    mutable size_t refcount;
};

template <typename T>
class Reference {
public:
    Reference() : object(nullptr) {}

    Reference(T* obj) : object(obj) {
        if (object) {
            object->addReference();
        }
    }

    Reference(const Reference& ref) : Reference((T*) ref) {}

    // We want to ensure that an implicit cast from Reference<X> to
    // Reference<Y> is only possible when also an implicit cast from X* to Y*
    // is possible. To achieve this we provide a constructor for the implicit
    // conversion and a conversion operator for the explicit conversion.
    // To ensure that the correct one is called, we need some template magic to
    // exclude the implicitly converting constructor from the overload set when
    // an implicit cast is not allowed.
    template <typename T2, typename = decltype(Reference<T>((T2*) nullptr))>
    Reference(const Reference<T2>& ref) : Reference((T2*) ref) {}

    template <typename T2>
    explicit operator Reference<T2>() const {
        return Reference<T2>((T2*) object);
    }

    Reference(Reference&& ref) {
        object = ref.object;
        ref.object = nullptr;
    }

    ~Reference() {
        if (object) {
            object->removeReference();
        }
    }

    Reference& operator=(const Reference& ref) {
        return operator=<T>(ref);
    }

    template <typename T2>
    Reference& operator=(const Reference<T2>& ref) {
        if (object == (T2*) ref) return *this;

        if (object) {
            object->removeReference();
        }
        object = (T2*) ref;
        if (object) {
            object->addReference();
        }

        return *this;
    }

    Reference& operator=(T* const& obj) {
        if (object == obj) return *this;

        if (object) {
            object->removeReference();
        }
        object = obj;
        if (object) {
            object->addReference();
        }

        return *this;
    }

    template <typename T2>
    bool operator==(const Reference<T2>& ref) const {
        return object == (T2*) ref;
    }

    template <typename T2>
    bool operator==(T2* const& obj) const {
        return object == obj;
    }

    template <typename T2>
    bool operator!=(const Reference<T2>& ref) const {
        return object != (T2*) ref;
    }

    template <typename T2>
    bool operator!=(T2* const& obj) const {
        return object != obj;
    }

    operator bool() const {
        return object;
    }

    explicit operator T*() const {
        return object;
    }

    T& operator*() const {
        return *object;
    }

    T* operator->() const {
        return object;
    }

private:
    T* object;
};

#endif
