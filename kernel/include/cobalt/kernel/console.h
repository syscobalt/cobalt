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

#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

#include <wchar.h>
#include <cobalt/kernel/display.h>
#include <cobalt/kernel/terminal.h>

#define MAX_PARAMS 16

class Console : public Terminal, public KeyboardListener {
public:
    Console();
    void lock();
    void unlock();
    void updateDisplaySize();
protected:
    bool getTtyPath(char* buffer, size_t size) override;
    void output(const char* buffer, size_t size);
private:
    void handleSequence(const char* sequence);
    void onKeyboardEvent(int key) override;
    void setGraphicsRendition();
    void printCharacter(char c);
    void printCharacterRaw(char c);
public:
    Reference<Display> display;
private:
    bool alternateBuffer;
    Color color;
    Color savedColor;
    Color alternateSavedColor;
    bool fgIsVgaColor;
    CharPos cursorPos;
    CharPos savedPos;
    CharPos alternateSavedPos;
    bool reversedColors;

    bool endOfLine;
    unsigned int params[MAX_PARAMS];
    bool paramSpecified[MAX_PARAMS];
    size_t paramIndex;
    mbstate_t ps;
    bool questionMarkModifier;
    int status;
};

extern Reference<Console> console;

#endif
