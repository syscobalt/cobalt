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

#ifndef _COBALT_KBKEYS_H
#define _COBALT_KBKEYS_H

#include <stddef.h>

#define KB_ESCAPE 0x01
#define KB_1 0x02
#define KB_2 0x03
#define KB_3 0x04
#define KB_4 0x05
#define KB_5 0x06
#define KB_6 0x07
#define KB_7 0x08
#define KB_8 0x09
#define KB_9 0x0A
#define KB_0 0x0B
#define KB_SYMBOL1 0x0C /* - */
#define KB_SYMBOL2 0x0D /* = */
#define KB_BACKSPACE 0x0E
#define KB_TAB 0x0F
#define KB_Q 0x10
#define KB_W 0x11
#define KB_E 0x12
#define KB_R 0x13
#define KB_T 0x14
#define KB_Y 0x15
#define KB_U 0x16
#define KB_I 0x17
#define KB_O 0x18
#define KB_P 0x19
#define KB_SYMBOL3 0x1A /* [ */
#define KB_SYMBOL4 0x1B /* ] */
#define KB_ENTER 0x1C
#define KB_LCONTROL 0x1D
#define KB_A 0x1E
#define KB_S 0x1F
#define KB_D 0x20
#define KB_F 0x21
#define KB_G 0x22
#define KB_H 0x23
#define KB_J 0x24
#define KB_K 0x25
#define KB_L 0x26
#define KB_SYMBOL5 0x27 /* ; */
#define KB_SYMBOL6 0x28 /* ' */
#define KB_SYMBOL7 0x29 /* ` */
#define KB_LSHIFT 0x2A
#define KB_SYMBOL8 0x2B /* \ */
#define KB_Z 0x2C
#define KB_X 0x2D
#define KB_C 0x2E
#define KB_V 0x2F
#define KB_B 0x30
#define KB_N 0x31
#define KB_M 0x32
#define KB_SYMBOL9 0x33 /* , */
#define KB_SYMBOL10 0x34 /* . */
#define KB_SYMBOL11 0x35 /* / */
#define KB_RSHIFT 0x36
#define KB_NUMPAD_MULT 0x37
#define KB_LALT 0x38
#define KB_SPACE 0x39
#define KB_CAPSLOCK 0x3A
#define KB_F1 0x3B
#define KB_F2 0x3C
#define KB_F3 0x3D
#define KB_F4 0x3E
#define KB_F5 0x3F
#define KB_F6 0x40
#define KB_F7 0x41
#define KB_F8 0x42
#define KB_F9 0x43
#define KB_F10 0x44
#define KB_NUMLOCK 0x45
#define KB_SCROLLLOCK 0x46
#define KB_NUMPAD7 0x47
#define KB_NUMPAD8 0x48
#define KB_NUMPAD9 0x49
#define KB_NUMPAD_MINUS 0x4A
#define KB_NUMPAD4 0x4B
#define KB_NUMPAD5 0x4C
#define KB_NUMPAD6 0x4D
#define KB_NUMPAD_PLUS 0x4E
#define KB_NUMPAD1 0x4F
#define KB_NUMPAD2 0x50
#define KB_NUMPAD3 0x51
#define KB_NUMPAD0 0x52
#define KB_NUMPAD_DOT 0x53

#define KB_SYMBOL12 0x56
#define KB_F11 0x57
#define KB_F12 0x58

#define KB_NUMPAD_ENTER 0x9C
#define KB_RCONTROL 0x9D
#define KB_NUMPAD_DIV 0xB5
#define KB_ALTGR 0xB8
#define KB_HOME 0xC7
#define KB_UP 0xC8
#define KB_PAGEUP 0xC9
#define KB_LEFT 0xCB
#define KB_RIGHT 0xCD
#define KB_END 0xCF
#define KB_DOWN 0xD0
#define KB_PAGEDOWN 0xD1
#define KB_INSERT 0xD2
#define KB_DELETE 0xD3
#define KB_LGUI 0xDB
#define KB_RGUI 0xDC

struct kbwc {
    int kb;
    wchar_t wc;
};

#endif
