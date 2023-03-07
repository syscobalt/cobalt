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

#ifndef KERNEL_HASHTABLE_H
#define KERNEL_HASHTABLE_H

#include <string.h>
#include <cobalt/kernel/kernel.h>

// The type T must have a function hashKey() returning a unique TKey. It also
// needs to have a member T* nextInHashTable that is managed by the hash table.
// An object can only be member of one hash table at a time. That way we can
// implement the hash table in a way that operations cannot fail.
template <typename T, typename TKey = size_t>
class HashTable {
public:
    HashTable(size_t capacity, T* buffer[]) {
        table = buffer;
        memset(table, 0, capacity * sizeof(T*));
        this->capacity = capacity;
    }

    void add(T* object) {
        size_t hash = object->hashKey() % capacity;
        object->nextInHashTable = table[hash];
        table[hash] = object;
    }

    T* get(TKey key) {
        size_t hash = key % capacity;

        T* obj = table[hash];
        while (obj) {
            if (obj->hashKey() == key) {
                return obj;
            }
            obj = obj->nextInHashTable;
        }

        return nullptr;
    }

    void remove(TKey key) {
        size_t hash = key % capacity;

        T* obj = table[hash];
        if (obj->hashKey() == key) {
            table[hash] = obj->nextInHashTable;
            return;
        }
        while (obj->nextInHashTable) {
            T* next = obj->nextInHashTable;
            if (next->hashKey() == key) {
                obj->nextInHashTable = next->nextInHashTable;
                return;
            }
            obj = next;
        }
    }
private:
    T** table;
    size_t capacity;
};

#endif
