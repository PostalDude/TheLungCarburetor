#include "communications.h"
#include "datamodel.h"

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

static bool ParseCommand(uint8_t* pData, uint8_t length)
{
    (void)pData;
    (void)length;
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
        if ((millis() - gTickPublish) > kPeriodCommPublish)
        {
            //*** Needs profiling
            Serial.println("PS1:"); Serial.println(gDataModel.fPressure_mmH2O[0], 2);
            Serial.println("PS2:"); Serial.println(gDataModel.fPressure_mmH2O[1], 2);
            Serial.println("RQP:"); Serial.println(gDataModel.fRequestPressure_mmH2O, 2);
            Serial.println("BAT:"); Serial.println(gDataModel.fBatteryLevel, 2);
            Serial.println("PMP:"); Serial.println(gDataModel.nPWMPump, DEC);
            Serial.println("STA:"); Serial.println(gDataModel.nState, DEC);
            Serial.println("CTL:"); Serial.println(gDataModel.nControlMode, DEC);
            Serial.println("TRG:"); Serial.println(gDataModel.nTriggerMode, DEC);
            Serial.println("CYC:"); Serial.println(gDataModel.nCycleState, DEC);

            gTickPublish = millis();
        }

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
                    ParseCommand(&gRxBuffer.data[cmdOfs], a+1 - cmdOfs);

                    ++a;
                    cmdOfs = a+1;
                }
            }

            if (cmdOfs > 0 && cmdOfs < count)
            {
                memmove(&gRxBuffer.data[0], &gRxBuffer.data[cmdOfs], count-cmdOfs);
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
