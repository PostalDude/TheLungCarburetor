#pragma once

#include <stdint.h>

class SerialPortReader
{
public:
    static bool ParseCommand(uint8_t* pData, uint8_t length);
};