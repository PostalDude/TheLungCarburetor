#include "serialportreader.h"

#include "common.h"
#include "configuration.h"
#include "datamodel.h"

namespace
{
    // Protocol debug2
    enum Commands
    {
        Commands_Unknown = 0,
        Commands_Status,
        Commands_Alive,
        Commands_Trigger,
        Commands_Control,
        Commands_Cycle,
        Commands_Fio,
        Commands_Curve,
        Commands_AlarmMinBatteryLevel,
        Commands_AlarmLowTidalVolume,
        Commands_AlarmHighTidalVolume,
        Commands_AlarmLowPressure,
        Commands_AlarmHighPressure,
        Commands_AlarmHighDeltaPressure,
        Commands_AlarmLowFio2Mix,
        Commands_AlarmHighFio2Mix,
        Commands_AlarmNonRebreathingValue,
        Commands_InitializePressureSensor,
        Commands_InitializePeepValue,
        Commands_InitializeTidalVolume,
        Commands_Count
    };

    const char* CommandsData[] = {
        "UNK",
        "STA",
        "ALI",
        "TRI",
        "CTL",
        "CYC",
        "FIO",
        "CUR",
        "MBL",
        "ALT",
        "AHT",
        "ALP",
        "AHP",
        "ADP",
        "ALF",
        "AHF",
        "ANR",
        "IPS",
        "IPV",
        "ITV",
        "UNK"
    };


    enum ReturnCommands
    {
        ReturnCommands_ACK,
        ReturnCommands_NACK,
        ReturnCommands_Count
    };

    const char* ReturnCommands[] = {
        "ACK\r\n",
        "NACK\r\n",
        "NACK\r\n" // in case someone uses the count item...
    };

    // Scratch Buffer to work on Array parsing
    enum eConsts
    {
        kCommandSize        = 3,
        kScratchBufferSize  = 128,
        kParseBufferSize    = 24
    };
    static uint8_t gScratchBuffer[kScratchBufferSize];
    static char    gParseBuffer[kParseBufferSize];


    template <typename T>
    bool getValue(const uint8_t* pData, uint8_t& index, const uint8_t length, T& value)
    {
        // We're using strings for now since the serial class in python can't seem to send the ints properly.
        uint8_t dataSize = sizeof(T);
        if (index >= length || index + dataSize >= length)
        {
            return false;
        }
        else
        {
            value = *((T*)& pData[index]);
            index += sizeof(T);
            return true;
        }
    }

    template <typename T>
    bool getValueArray(const uint8_t* pData, uint8_t& index, const uint8_t length, T*& value, int32_t& count)
    {
        value = nullptr;

        if (index >= length)
        {
            return false;
        }

        if (!getValue(pData, index, length, count))
        {
            return false;
        }

        if (count <= 0)
        {
            return false;
        }

        if (index + (count * sizeof(T)) >= length)
        {
            return false;
        }

        value = (T*)&gScratchBuffer[0];
        if (count * sizeof(T) > kScratchBufferSize)
        {
            value = nullptr;
            return false;
        }

        memset(value, 0, sizeof(T) * count);
        for (int32_t n = 0; n < count; ++n)
        {
            if (!getValue(pData, index, length, value[n]))
            {
                value = nullptr;
                return false;
            }
        }

        return true;
    }

    // DEBUG function
    void printValue(const char* str, int f)
    {
        char b[30];
        itoa(f, b, 10);
        Serial.print(str); Serial.println(b);
    }
    // printValue("DEBUG: in ratio ", (int)(inhaleRatio*1000.0f));
}

bool SerialPortReader::ParseCommand(uint8_t* pData, uint8_t length)
{
    // replace the end of the string with 0s so it's compatible with the strcmp function
    uint8_t dataIndex = 0;
    
    // First bytes is the command
    if (length < kCommandSize)
    {
        return false;
    }

    uint8_t command[kCommandSize + 1] = { pData[dataIndex++], pData[dataIndex++], pData[dataIndex++], '\0' };

    uint8_t commandIndex = 0;
    for (commandIndex = 1; commandIndex < Commands_Count; ++commandIndex)
    {
        if (strcmp(reinterpret_cast<char*>(command), CommandsData[commandIndex]) == 0)
        {
            // we found our command!
            break;
        }
    }

    switch (commandIndex)
    {
    case Commands_Alive:
        Serial.print(ReturnCommands[ReturnCommands_ACK]);
        break;

    case Commands_Status: 
    {
        dtostrf(gDataModel.fPressure_mmH2O[0],0, 5, gParseBuffer);
        Serial.print("PS1:"); Serial.print(gParseBuffer);
        dtostrf(gDataModel.fPressure_mmH2O[1],0, 5, gParseBuffer);
        Serial.print(",PS2:"); Serial.print(gParseBuffer);
        dtostrf(gDataModel.fRequestPressure_mmH2O,0, 5, gParseBuffer);
        Serial.print(",RPQ:"); Serial.print(gParseBuffer);
        dtostrf(gDataModel.fBatteryLevel,0, 5, gParseBuffer);
        Serial.print(",BAT:"); Serial.print(gParseBuffer);
        itoa(static_cast<int>(gDataModel.nPWMPump), gParseBuffer, 10);
        Serial.print(",PMP:"); Serial.print(gParseBuffer);
        itoa(static_cast<int>(gDataModel.nState), gParseBuffer, 10);
        Serial.print(",STA:"); Serial.print(gParseBuffer);
        itoa(static_cast<int>(gDataModel.nControlMode), gParseBuffer, 10);
        Serial.print(",CTL:"); Serial.print(gParseBuffer);
        itoa(static_cast<int>(gDataModel.nTriggerMode), gParseBuffer, 10);
        Serial.print(",TRG:"); Serial.print(gParseBuffer);
        itoa(static_cast<int>(gDataModel.nCycleState), gParseBuffer, 10);
        Serial.print(",CYC:"); Serial.print(gParseBuffer);

        // Alarms
        if (gDataModel.nSafetyFlags & kAlarm_MinPressureLimit)
            Serial.print(",ALARM:MIM_PRESSURE_LIMIT");
        if (gDataModel.nSafetyFlags & kAlarm_MaxPressureLimit)
            Serial.print(",ALARM:MAX_PRESSURE_LIMIT");
        if (gDataModel.nSafetyFlags & kAlarm_PressureSensorRedudancyFail)
            Serial.print(",ALARM:PRESSURE_SENSOR_REDUDANCY_FAIL");
        if (gDataModel.nSafetyFlags & kAlarm_InvalidConfiguration)
            Serial.print(",ALARM:INVALID_CONFIGURATION");

        Serial.print("\r\n");
    }
    break;

    case Commands_Control:
    {
        int32_t temp = 0;
        bool ok = getValue(pData, dataIndex, length, temp);
        if (ok)
            ok = temp < kControlMode_Count;
        if (ok)
        {
            gDataModel.nControlMode = static_cast<eControlMode>(temp);
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        }
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_Trigger:
    {
        int32_t temp = 0;
        bool ok = getValue(pData, dataIndex, length, temp);
        if (ok)
            ok = temp < kTriggerMode_Count;
        if (ok)
        {
            gDataModel.nTriggerMode = static_cast<eTriggerMode>(temp);
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        }
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_Cycle:
    {
        int8_t temp = 0;
        bool ok = getValue(pData, dataIndex, length, temp);
        if (ok)
        {
            gDataModel.bStartFlag = temp != 0;
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        }
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_Fio:
    {
        float fio;
        if (getValue(pData, dataIndex, length, fio) && fio >= 20.0f && fio <= 100.0f)
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_Curve:
    {
        float breatheRate = 0.0f;
        float inhaleMmH2O = 0.0f;
        float exhaleMmH2O = 0.0f;
        float inhaleRatio = 0.0f;
        float exhaleRatio = 0.0f;
        
        bool ok = getValue(pData, dataIndex, length, breatheRate);
        if (ok) ok = getValue(pData, dataIndex, length, inhaleMmH2O);
        if (ok) ok = getValue(pData, dataIndex, length, exhaleMmH2O);
        if (ok) ok = getValue(pData, dataIndex, length, inhaleRatio);
        if (ok) ok = getValue(pData, dataIndex, length, exhaleRatio);
        // Validate the inputs.
        if (ok)
        {
            ok = (
                inhaleMmH2O >= 0.0f && inhaleMmH2O <= 40.0f &&
                exhaleMmH2O >= 0.0f && exhaleMmH2O <= 25.0f &&
                inhaleRatio >= 0.0f && exhaleRatio >= 0.0f &&
                breatheRate >= 6.0f && breatheRate <= 40.0f
            );
        }

        if (ok)
        {
            gDataModel.nRespirationPerMinute = breatheRate;

            // Updating Data model
            float breatheTime = 1.0f/breatheRate; //TODO: Pass breathe time instead of breathre Rate - or better yet, separate inhale and exhale times

            //Inhale curve
            tPressureCurve inhaleCurve;
            inhaleCurve.nCount = 3; //Initial point, flex point, end point
            inhaleCurve.nSetPoint_TickMs[0] = 0;
            inhaleCurve.nSetPoint_TickMs[2] = static_cast<uint32_t>((breatheTime*inhaleRatio) * 1000); //Assumes inhaleRatio + exhaleRatio = 1
            inhaleCurve.nSetPoint_TickMs[1] = inhaleCurve.nSetPoint_TickMs[2] / 2; //I don't know if it's better to convert or to round.

            inhaleCurve.fSetPoint_mmH2O[0] = exhaleMmH2O;
            inhaleCurve.fSetPoint_mmH2O[1] = inhaleMmH2O;
            inhaleCurve.fSetPoint_mmH2O[2] = inhaleMmH2O;
            //TODO: Add more intermediary points if curve is not smooth enough

            //Exhale curve
            tPressureCurve exhaleCurve;
            exhaleCurve.nCount = 3;
            exhaleCurve.nSetPoint_TickMs[0] = 0;
            exhaleCurve.nSetPoint_TickMs[2] = static_cast<uint32_t>((breatheTime*exhaleRatio) * 1000); //Assumes inhaleRatio + exhaleRatio = 1
            exhaleCurve.nSetPoint_TickMs[1] = exhaleCurve.nSetPoint_TickMs[2] / 2; //I don't know if it's better to convert or to round.

            exhaleCurve.fSetPoint_mmH2O[0] = inhaleMmH2O;
            exhaleCurve.fSetPoint_mmH2O[1] = exhaleMmH2O;
            exhaleCurve.fSetPoint_mmH2O[2] = exhaleMmH2O;

            gDataModel.pInhaleCurve = inhaleCurve;
            gDataModel.pExhaleCurve = exhaleCurve;
            
            /*********** IMPORTANT NOTE *******************/
            /*
            Depending on how this profile is read to send commands, this could cause the system to output something dangerous. I don't know where the profile is handled in the code.

            Expected correct way (pseudo code):

            while (1)
            if t between CurrentCurve.TickMS[0] and currentCurve.TickMS[1]
                cmd = CurrentCurve.fSetPoint_mmH2O[1];

            else if t between CurrentCurve.TickMS[1] and currentCurve.TickMS[2]
                cmd = CurrentCurve.fSetPoint_mmH2O[2];

            t++


            Possible seemingly logical way that would yield the wrong profile

            while(1)
                for (i = 1; i<CurrentCurve.nCount; i++)
                    if (t == CurrentCurve.TickMS[i])
                        currentCmd = CurrentCurve.fSetPoint_mmH2O[i];
            */
        }

        if (ok)
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmMinBatteryLevel:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
        {
            if (temp >= 0.1f) // TODO
            {
                gConfiguration.fMinBatteryLevel = temp;
                Serial.print(ReturnCommands[ReturnCommands_ACK]);
            }
            else
                Serial.print(ReturnCommands[ReturnCommands_NACK]);
        }
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmLowTidalVolume:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmHighTidalVolume:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmLowPressure:
    {
        if (getValue(pData, dataIndex, length, gConfiguration.fMinPressureLimit_mmH2O))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmHighPressure:
    {
        if (getValue(pData, dataIndex, length, gConfiguration.fMaxPressureLimit_mmH2O))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmHighDeltaPressure:
    {
        if (getValue(pData, dataIndex, length, gConfiguration.fMaxPressureDelta_mmH2O))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmLowFio2Mix:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmHighFio2Mix:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_AlarmNonRebreathingValue:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;

    case Commands_InitializePressureSensor:
    {
        gConfiguration.nPressureSensorOffset[0] = gDataModel.nRawPressure[0];
        gConfiguration.nPressureSensorOffset[1] = gDataModel.nRawPressure[1];
        Serial.print(ReturnCommands[ReturnCommands_ACK]);
    }
    break;

    case Commands_InitializePeepValue:
    {
        Serial.print(ReturnCommands[ReturnCommands_ACK]);
    }
    break;

    case Commands_InitializeTidalVolume:
    {
        Serial.print(ReturnCommands[ReturnCommands_ACK]);
    }
    break;
    
    default:
        Serial.print(ReturnCommands[ReturnCommands_NACK]);
        break;
    }

    return true;
}
