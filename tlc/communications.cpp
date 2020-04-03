///
/// \file       communications.cpp
/// \brief      The Lung Carburetor Firmware communications module
///
/// \author     Frederic Lauzon
/// \ingroup    communications
#include "communications.h"
#include "datamodel.h"
#include "lcd_keypad.h"
#include "serialportreader.h"

static bool     gSerialConnected    = false;

/// \struct tRxBuffer
/// \brief Receive buffer state
struct tRxBuffer
{
    uint8_t     data[kRxBufferSize];    ///> Data receive on serial port
    uint8_t     rxSize;                 ///> Size of data buffer
    uint32_t    lastRxTick;             ///> Last reception tick
};
static tRxBuffer gRxBuffer;


bool Communications_Init()
{
    gSerialConnected        = false;

    gRxBuffer.rxSize        = 0;
    gRxBuffer.lastRxTick    = millis();

    Serial.begin(kSerialBaudRate);

    return true;
}

void Communications_Process()
{
    // Communications is allowed in all states
    if (!gSerialConnected)
    {
        if (Serial)
        {
            Serial.setTimeout(kSerialRxTimeOut);
            gSerialConnected = true;
        }
    }
    else
    {
        if (Serial.available() > 0)
        {
            if ((millis() - gRxBuffer.lastRxTick) > kSerialDiscardTimeout)
            {
                gRxBuffer.rxSize = 0;
            }

            int ofs   = gRxBuffer.rxSize;
            int count = gRxBuffer.rxSize + Serial.available();
            if (count >= kRxBufferSize)
            {
                count = kRxBufferSize-1;
            }

            Serial.readBytes(&gRxBuffer.data[ofs], count-ofs);

            // Scan for crlf
            int cmdOfs = 0;
            for (int a = 0; a < count-1; ++a)
            {
                if (gRxBuffer.data[a]   == '\r' &&
                    gRxBuffer.data[a+1] == '\n')
                {
                    ParseCommand(&gRxBuffer.data[cmdOfs], a+2 - cmdOfs);

                    ++a;
                    cmdOfs = a+1;
                }
            }

            if (cmdOfs > 0 && cmdOfs <= count)
            {
                memmove(&gRxBuffer.data[0], &gRxBuffer.data[cmdOfs], cmdOfs);
                count = count - cmdOfs;
            }

            // Discard data if we have too much in bank
            if (count > kRxBufferSize - kRxBufferReserve)
            {
                count = 0;
            }

            gRxBuffer.rxSize = count;
            gRxBuffer.lastRxTick = millis();
        }

        #ifdef PRINT_DEBUG_TO_SERIAL
        // Print the lcd details on the serial since not everyone has one!
        if (strlen(gLcdDetail) > 0)
        {
            Serial.print("DEBUG:");
            Serial.println(gLcdDetail);
        }
        #endif
    }
}
