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
        Commands_Configs,
        Commands_Status,
        Commands_Alive,
        Commands_Trigger,
        Commands_Control,
        Commands_Cycle,
        Commands_Fio,
        Commands_Curve,
        Commands_TakeOverThreshold,
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
        "CFG",
        "STA",
        "ALI",
        "TRI",
        "CTL",
        "CYC",
        "FIO",
        "CUR",
        "TTH",
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

    template <typename T>
    void serialPrint(T t);

    template <>
    void serialPrint(float t)
    {
        dtostrf(t,0, 5, gParseBuffer);
        Serial.print(gParseBuffer);
    }

    template <>
    void serialPrint(int  t)
    {
        itoa(t, gParseBuffer, 10);
        Serial.print(gParseBuffer);
    }

    // DEBUG function
    void printValue(const char* str, int f)
    {
        itoa(f, gParseBuffer, 10);
        Serial.print(str); Serial.println(gParseBuffer);
    }
    // printValue("DEBUG: in ratio ", (int)(inhaleRatio*1000.0f));
}

void updateCurve()
{
    // Updating Data model
    float breatheTime = 1.0f/gDataModel.nRespirationPerMinute; //TODO: Pass breathe time instead of breathre Rate - or better yet, separate inhale and exhale times

    //Inhale curve
    gDataModel.pInhaleCurve.nCount = 3; //Initial point, flex point, end point
    gDataModel.pInhaleCurve.nSetPoint_TickMs[0] = 0;
    gDataModel.pInhaleCurve.nSetPoint_TickMs[2] = static_cast<uint32_t>((breatheTime*gDataModel.fInhaleRatio) * 1000); //Assumes inhaleRatio + exhaleRatio = 1
    gDataModel.pInhaleCurve.nSetPoint_TickMs[1] = gDataModel.pInhaleCurve.nSetPoint_TickMs[2] / 2; //I don't know if it's better to convert or to round.

    gDataModel.pInhaleCurve.fSetPoint_mmH2O[0] = gDataModel.fExhalePressureTarget_mmH2O;
    gDataModel.pInhaleCurve.fSetPoint_mmH2O[1] = gDataModel.fInhalePressureTarget_mmH2O;
    gDataModel.pInhaleCurve.fSetPoint_mmH2O[2] = gDataModel.fInhalePressureTarget_mmH2O;
    //TODO: Add more intermediary points if curve is not smooth enough

    //Exhale curve
    gDataModel.pExhaleCurve.nCount = 3;
    gDataModel.pExhaleCurve.nSetPoint_TickMs[0] = 0;
    gDataModel.pExhaleCurve.nSetPoint_TickMs[2] = static_cast<uint32_t>((breatheTime*gDataModel.fExhaleRatio) * 1000); //Assumes inhaleRatio + exhaleRatio = 1
    gDataModel.pExhaleCurve.nSetPoint_TickMs[1] = gDataModel.pExhaleCurve.nSetPoint_TickMs[2] / 2; //I don't know if it's better to convert or to round.

    gDataModel.pExhaleCurve.fSetPoint_mmH2O[0] = gDataModel.fInhalePressureTarget_mmH2O;
    gDataModel.pExhaleCurve.fSetPoint_mmH2O[1] = gDataModel.fExhalePressureTarget_mmH2O;
    gDataModel.pExhaleCurve.fSetPoint_mmH2O[2] = gDataModel.fExhalePressureTarget_mmH2O;
    
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

bool ParseCommand(uint8_t* pData, uint8_t length)
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
    case Commands_Configs:
    {
        serialPrint(0.0f); // FIO
        Serial.print(","); serialPrint(0.0f);//serialPrint(gConfiguration.fTakeOverThreshold_ms);
        float breatheRate = static_cast<float>(gDataModel.nRespirationPerMinute);
        Serial.print(","); serialPrint(breatheRate); 
        Serial.print(","); serialPrint(gDataModel.fInhalePressureTarget_mmH2O);
        Serial.print(","); serialPrint(gDataModel.fExhalePressureTarget_mmH2O);
        Serial.print(","); serialPrint(gDataModel.fInhaleRatio);
        Serial.print(","); serialPrint(gDataModel.fExhaleRatio);
        Serial.print(","); serialPrint(gConfiguration.fMinBatteryLevel);
        Serial.print(","); serialPrint(0.0f); // ALT - alarm low tidal
        Serial.print(","); serialPrint(0.0f); // AHT - alarm high tidal
        Serial.print(","); serialPrint(gConfiguration.fMinPressureLimit_mmH2O);
        Serial.print(","); serialPrint(gConfiguration.fMaxPressureLimit_mmH2O);
        Serial.print(","); serialPrint(gConfiguration.fMaxPressureDelta_mmH2O);
        Serial.print(","); serialPrint(0.0f); // ALF - alarm low fio mix
        Serial.print(","); serialPrint(0.0f); // AHF - alarm high fio mix
        Serial.print(","); serialPrint(0.0f); // ANR - alarm non rebreathing value

        Serial.print("\r\n");
    }
    break;

    case Commands_Alive:
        Serial.println("ACK");
        break;

    case Commands_Status: 
    {
        serialPrint(gDataModel.fPressure_mmH2O[0]);
        Serial.print(","); serialPrint(gDataModel.fPressure_mmH2O[1]);
        Serial.print(","); serialPrint(gDataModel.fRequestPressure_mmH2O);
        Serial.print(","); serialPrint(gDataModel.fBatteryLevel);
        Serial.print(","); serialPrint(static_cast<int>(gDataModel.nPWMPump));
        Serial.print(","); serialPrint(static_cast<int>(gDataModel.nState));
        Serial.print(","); serialPrint(static_cast<int>(gDataModel.nControlMode));
        Serial.print(","); serialPrint(static_cast<int>(gDataModel.nTriggerMode));
        Serial.print(","); serialPrint(static_cast<int>(gDataModel.nCycleState));

        // Alarms
        (gDataModel.nSafetyFlags & kAlarm_MinPressureLimit) ? Serial.print(",1") : Serial.print(",0");
        (gDataModel.nSafetyFlags & kAlarm_MaxPressureLimit) ? Serial.print(",1") : Serial.print(",0");
        (gDataModel.nSafetyFlags & kAlarm_PressureSensorRedudancyFail) ? Serial.print(",1") : Serial.print(",0");
        (gDataModel.nSafetyFlags & kAlarm_InvalidConfiguration) ? Serial.print(",1") : Serial.print(",0");
        
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
            Serial.println("ACK");
        }
        else
            Serial.println("NACK");
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
            Serial.println("ACK");
        }
        else
            Serial.println("NACK");
    }
    break;

    case Commands_Cycle:
    {
        int8_t temp = 0;
        bool ok = getValue(pData, dataIndex, length, temp);
        if (ok)
        {
            gDataModel.bStartFlag = temp != 0;
            Serial.println("ACK");
        }
        else
            Serial.println("NACK");
    }
    break;

    case Commands_Fio:
    {
        float fio;
        if (getValue(pData, dataIndex, length, fio) && fio >= 20.0f && fio <= 100.0f)
            Serial.println("ACK");
        else
            Serial.println("NACK");
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
            gDataModel.nRespirationPerMinute = static_cast<uint8_t>(breatheRate);
            gDataModel.fInhalePressureTarget_mmH2O = inhaleMmH2O;
            gDataModel.fExhalePressureTarget_mmH2O = exhaleMmH2O;
            gDataModel.fInhaleRatio             = inhaleRatio;
            gDataModel.fExhaleRatio             = exhaleRatio;
            updateCurve();
        }

        if (ok)
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_TakeOverThreshold:
    {
        int32_t temp;
        if (getValue(pData, dataIndex, length, temp))
        {
            if (temp >= 0.0f)
            {
                //gConfiguration.fTakeOverThreshold_ms = temp;
                Serial.println("ACK");
            }
            else
                Serial.println("NACK");
        }
        else
            Serial.println("NACK");
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
                Serial.println("ACK");
            }
            else
                Serial.println("NACK");
        }
        else
            Serial.println("NACK");
    }
    break;

    case Commands_AlarmLowTidalVolume:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_AlarmHighTidalVolume:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_AlarmLowPressure:
    {
        if (getValue(pData, dataIndex, length, gConfiguration.fMinPressureLimit_mmH2O))
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_AlarmHighPressure:
    {
        if (getValue(pData, dataIndex, length, gConfiguration.fMaxPressureLimit_mmH2O))
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_AlarmHighDeltaPressure:
    {
        if (getValue(pData, dataIndex, length, gConfiguration.fMaxPressureDelta_mmH2O))
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_AlarmLowFio2Mix:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_AlarmHighFio2Mix:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_AlarmNonRebreathingValue:
    {
        float temp;
        if (getValue(pData, dataIndex, length, temp))
            Serial.println("ACK");
        else
            Serial.println("NACK");
    }
    break;

    case Commands_InitializePressureSensor:
    {
        gConfiguration.nPressureSensorOffset[0] = gDataModel.nRawPressure[0];
        gConfiguration.nPressureSensorOffset[1] = gDataModel.nRawPressure[1];
        Serial.println("ACK");
    }
    break;

    case Commands_InitializePeepValue:
    {
        Serial.println("ACK");
    }
    break;

    case Commands_InitializeTidalVolume:
    {
        Serial.println("ACK");
    }
    break;
    
    default:
        Serial.println("NACK");
        break;
    }

    return true;
}
