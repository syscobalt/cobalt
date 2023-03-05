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

#ifndef KERNEL_DISPLAY_H
#define KERNEL_DISPLAY_H

#include <cobalt/display.h>
#include <cobalt/kernel/vnode.h>

struct CharPos {
    unsigned int x;
    unsigned int y;

    bool operator==(const CharPos& p) { return p.x == x && p.y == y; }
};

struct Color {
    uint32_t fgColor;
    uint32_t bgColor;
    uint8_t vgaColor;

    bool operator!=(const Color& other) {
        return fgColor != other.fgColor || bgColor != other.bgColor ||
                vgaColor != other.vgaColor;
    }
};

struct CharBufferEntry {
    wchar_t wc;
    Color color;
    bool modified;

    bool operator!=(const CharBufferEntry& other) {
        return wc != other.wc || color != other.color;
    }
};

class Process;

class Display : public Vnode {
public:
    Display(video_mode mode, char* buffer, size_t pitch);
    void clear(CharPos from, CharPos to, Color color);
    int devctl(int command, void* restrict data, size_t size,
            int* restrict info) override;
    video_mode getVideoMode();
    void initialize();
    void onPanic();
    void putCharacter(CharPos position, wchar_t c, Color color);
    void releaseDisplay();
    void scroll(unsigned int lines, Color color, bool up = true);
    void setCursorPos(CharPos position);
    void setCursorVisibility(bool visible);
    int setVideoMode(video_mode* videoMode);
    void switchBuffer(Color color);
    void update();
private:
    char* charAddress(CharPos position);
    void redraw(CharPos position);
    void redraw(CharPos position, CharBufferEntry* entry);
    void setPixelColor(char* addr, uint32_t rgbColor);
    int setVideoModeUnlocked(video_mode* videoMode);
public:
    unsigned int columns;
    unsigned int rows;
private:
    char* buffer;
    video_mode mode;
    size_t pitch;
    CharPos cursorPos;
    bool cursorVisible;
    CharBufferEntry* doubleBuffer;
    CharBufferEntry* primaryBuffer;
    CharBufferEntry* alternateBuffer;
    bool invalidated;
    bool renderingText;
    bool haveOldBuffer;
    bool changingResolution;
    Process* displayOwner;
};

class GraphicsDriver {
public:
    virtual bool isSupportedMode(video_mode mode) = 0;
    virtual vaddr_t setVideoMode(video_mode* mode) = 0;
};

extern GraphicsDriver* graphicsDriver;

#endif
