#include "communications.h"
#include "datamodel.h"

static bool     gSerialConnected    = false;
static uint32_t gTickPublish        = 0;
static uint8_t  gMsgTxID            = 0;
static bool     gSendMsg            = false;

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
        if(gSendMsg)
        {
            //*** Needs profiling
            // Send only one message
            switch(gMsgTxID)
            {
              case 0:
                Serial.print("PS1:"); Serial.println(gDataModel.fPressure_mmH2O[0], 2);
                break;
              case 1:
                Serial.print("PS2:"); Serial.println(gDataModel.fPressure_mmH2O[1], 2);
                break;
              case 2:
                Serial.print("RQP:"); Serial.println(gDataModel.fRequestPressure_mmH2O, 2);
                break;
              case 3:
                Serial.print("BAT:"); Serial.println(gDataModel.fBatteryLevel, 2);
                break;
              case 4:
                Serial.print("PMP:"); Serial.println(gDataModel.nPWMPump, DEC);
                break;
              case 5:
                Serial.print("STA:"); Serial.println(gDataModel.nState, DEC);
                break;
              case 6:
                Serial.print("CTL:"); Serial.println(gDataModel.nControlMode, DEC);
                break;
              case 7:
                  Serial.print("TRG:"); Serial.println(gDataModel.nTriggerMode, DEC);
                  break;
              case 8:
                  Serial.print("CYC:"); Serial.println(gDataModel.nCycleState, DEC);
                  break;
              default:
                  gSendMsg = false;
            }

            // Select next message
            gMsgTxID++;

        } else if ((millis() - gTickPublish) > kPeriodCommPublish)
        {
          gSendMsg = true;
          gMsgTxID = 0;
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
