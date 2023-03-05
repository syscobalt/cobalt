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

#include <wchar.h>
#include <cobalt/kbkeys.h>
#include <cobalt/kernel/keyboard.h>

#define KBLAYOUT KBLAYOUT_US

// US keyboard layout.
static const wchar_t KBLAYOUT_US[] = {
    // no modifiers, shift, caps, unused
    0, 0, 0, 0,
    L'\e', L'\e', L'\e', L'\e',
    L'1', L'!', L'1', 0,
    L'2', L'@', L'2', 0,
    L'3', L'#', L'3', 0,
    L'4', L'$', L'4', 0,
    L'5', L'%', L'5', 0,
    L'6', L'^', L'6', 0,
    L'7', L'&', L'7', 0,
    L'8', L'*', L'8', 0,
    L'9', L'(', L'9', 0,
    L'0', L')', L'0', 0,
    L'-', L'_', L'-', 0,
    L'=', L'+', L'=', 0,
    L'\b', L'\b', L'\b', L'\b',
    L'\t', L'\t', L'\t', L'\t',
    L'q', L'Q', L'Q', 0,
    L'w', L'W', L'W', 0,
    L'e', L'E', L'E', 0,
    L'r', L'R', L'R', 0,
    L't', L'T', L'T', 0,
    L'y', L'Y', L'Y', 0,
    L'u', L'U', L'U', 0,
    L'i', L'I', L'I', 0,
    L'o', L'O', L'O', 0,
    L'p', L'P', L'P', 0,
    L'[', L'{', L'[', 0,
    L']', L'}', L']', 0,
    L'\n', L'\n', L'\n', L'\n',
    0, 0, 0, 0, // left Control
    L'a', L'A', L'A', 0,
    L's', L'S', L'S', 0,
    L'd', L'D', L'D', 0,
    L'f', L'F', L'F', 0,
    L'g', L'G', L'G', 0,
    L'h', L'H', L'H', 0,
    L'j', L'J', L'J', 0,
    L'k', L'K', L'K', 0,
    L'l', L'L', L'L', 0,
    L';', L':', L';', 0,
    L'\'', L'"', L'\'', 0,
    L'`', L'~', L'`', 0,
    0, 0, 0, 0, // left Shift
    L'\\', L'|', L'\\', 0,
    L'z', L'Z', L'Z', 0,
    L'x', L'X', L'X', 0,
    L'c', L'C', L'C', 0,
    L'v', L'V', L'V', 0,
    L'b', L'B', L'B', 0,
    L'n', L'N', L'N', 0,
    L'm', L'M', L'M', 0,
    L',', L'<', L',', 0,
    L'.', L'>', L'.', 0,
    L'/', L'?', L'/', 0,
    0, 0, 0, 0, // right Shift
    L'*', L'*', L'*', L'*',
    0, 0, 0, 0, // left Alt
    L' ', L' ', L' ', L' ',
    0, 0, 0, 0, // Caps Lock
    0, 0, 0, 0, // F1
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0, // F10
    0, 0, 0, 0, // Num Lock
    0, 0, 0, 0, // Scroll Lock
    L'7', 0, L'7', L'7',
    L'8', 0, L'8', L'8',
    L'9', 0, L'9', L'9',
    L'-', L'-', L'-', L'-',
    L'4', 0, L'4', L'4',
    L'5', 0, L'5', L'5',
    L'6', 0, L'6', L'6',
    L'+', L'+', L'+', L'+',
    L'1', 0, L'1', L'1',
    L'2', 0, L'2', L'2',
    L'3', 0, L'3', L'3',
    L'0', 0, L'0', L'0',
    L'.', 0, L'.', L'.',
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0, // F11
    0, 0, 0, 0, // F12
    // Most things below are not printable
};

static const struct {
    int key;
    const char* sequence;
} sequences[] = {
    { KB_UP, "\e[A" },
    { KB_DOWN, "\e[B" },
    { KB_RIGHT, "\e[C" },
    { KB_LEFT, "\e[D" },
    { KB_END, "\e[F" },
    { KB_HOME, "\e[H" },
    { KB_INSERT, "\e[2~" },
    { KB_DELETE, "\e[3~" },
    { KB_PAGEUP, "\e[5~" },
    { KB_PAGEDOWN, "\e[6~" },
    { KB_F1, "\e[OP" },
    { KB_F2, "\e[OQ" },
    { KB_F3, "\e[OR" },
    { KB_F4, "\e[OS" },
    { KB_F5, "\e[15~" },
    { KB_F6, "\e[17~" },
    { KB_F7, "\e[18~" },
    { KB_F8, "\e[19~" },
    { KB_F9, "\e[20~" },
    { KB_F10, "\e[21~" },
    { KB_F11, "\e[23~" },
    { KB_F12, "\e[24~" },
    { 0, 0 }
};

wchar_t Keyboard::getWideCharFromKey(int key) {
    static bool leftShift = false;
    static bool rightShift = false;
    static bool capsLock = false;
    static bool leftControl = false;
    static bool rightControl = false;

    if (key == KB_LSHIFT) {
        leftShift = true;
    } else if (key == KB_RSHIFT) {
        rightShift = true;
    } else if (key == KB_CAPSLOCK) {
        capsLock = !capsLock;
    } else if (key == KB_LCONTROL) {
        leftControl = true;
    } else if (key == KB_RCONTROL) {
        rightControl = true;
    } else if (key == -KB_LSHIFT) {
        leftShift = false;
    } else if (key == -KB_RSHIFT) {
        rightShift = false;
    } else if (key == -KB_LCONTROL) {
        leftControl = false;
    } else if (key == -KB_RCONTROL) {
        rightControl = false;
    }

    if (key < 0) return L'\0';

    wchar_t wc = L'\0';
    if ((size_t) key < sizeof(KBLAYOUT) / sizeof(wchar_t) / 4) {
        size_t index = key << 2;
        bool shift = leftShift || rightShift;
        if (shift && capsLock) {
            // When shift and caps have the same effect they cancel each other.
            if (KBLAYOUT[index + 1] != KBLAYOUT[index + 2]) {
                index++;
            }
        } else if (shift) {
            index++;
        } else if (capsLock) {
            index += 2;
        }
        wc = KBLAYOUT[index];
    } else if (key == KB_NUMPAD_ENTER) {
        wc = L'\n';
    } else if (key == KB_NUMPAD_DIV) {
        wc = L'/';
    }

    bool control = leftControl || rightControl;
    if (control && wc >= L'@' && wc <= L'~') {
        wc = wc & 0x1F;
    } else if (control && wc == L'?') {
        wc = L'\x7F';
    }

    return wc;
}

const char* Keyboard::getSequenceFromKey(int key) {
    for (size_t i = 0; sequences[i].key != 0; i++) {
        if (sequences[i].key == key) {
            return sequences[i].sequence;
        }
    }

    return NULL;
}
