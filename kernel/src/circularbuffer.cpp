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

#include <string.h>
#include <cobalt/kernel/circularbuffer.h>

CircularBuffer::CircularBuffer() {

}

CircularBuffer::CircularBuffer(char* buffer, size_t size) {
    initialize(buffer, size);
}

void CircularBuffer::initialize(char* buffer, size_t size) {
    this->buffer = buffer;
    bufferSize = size;
    readPosition = 0;
    bytesStored = 0;
}

size_t CircularBuffer::bytesAvailable() {
    return bytesStored;
}

size_t CircularBuffer::spaceAvailable() {
    return bufferSize - bytesStored;
}

size_t CircularBuffer::read(void* buf, size_t size) {
    size_t bytesRead = 0;
    while (bytesStored > 0 && bytesRead < size) {
        size_t count = bufferSize - readPosition;
        if (count > size - bytesRead) count = size - bytesRead;
        if (count > bytesStored) count = bytesStored;

        memcpy((char*) buf + bytesRead, buffer + readPosition, count);
        readPosition = (readPosition + count) % bufferSize;
        bytesStored -= count;
        bytesRead += count;
    }
    return bytesRead;
}

size_t CircularBuffer::write(const void* buf, size_t size) {
    size_t written = 0;
    while (spaceAvailable() > 0 && written < size) {
        size_t writeIndex = (readPosition + bytesStored) % bufferSize;
        size_t count = bufferSize - writeIndex;
        if (count > size - written) count = size - written;
        if (count > spaceAvailable()) count = spaceAvailable();

        memcpy(buffer + writeIndex, (char*) buf + written, count);
        written += count;
        bytesStored += count;
    }
    return written;
}
