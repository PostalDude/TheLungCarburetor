#include "communications.h"
#include "datamodel.h"
#include "serialportreader.h"

static bool     gSerialConnected    = false;
static uint32_t gTickPublish        = 0;

struct tRxBuffer
{
    uint8_t     data[kRxBufferSize];
    uint8_t     rxSize;
    uint32_t    lastRxTick;
};
static tRxBuffer gRxBuffer;


bool Communications_Init()
{
    gSerialConnected        = false;
    gTickPublish            = millis();

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
        // Process input string, wait for AT .... <cr><lf>
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
            Serial.readBytes(&gRxBuffer.data[ofs], count);

            // Scan for crlf
            int cmdOfs = 0;
            for (int a = 0; a < count-1; ++a)
            {
                if (gRxBuffer.data[a]   == '\r' &&
                    gRxBuffer.data[a+1] == '\n')
                {
                    SerialPortReader::ParseCommand(&gRxBuffer.data[cmdOfs], a+2 - cmdOfs);

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
    }
}
