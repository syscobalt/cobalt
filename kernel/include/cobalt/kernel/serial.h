#pragma once

#define COM1 0x3F8

namespace Serial {
    void initialize();
    int serialRecieved();
    int isTransmitEmpty();
    void writeSerial(char);
};