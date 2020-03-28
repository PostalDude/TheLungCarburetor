#include "serialportreader.h"

#include "common.h"
#include "datamodel.h"

namespace
{
    enum Commands
    {
        Commands_Unknown = 0,
        Commands_Status,
        Commands_Alive,
        Commands_Trigger,
        Commands_Fio,
        Commands_Curve,
        Commands_InhalePsi,
        Commands_ExhalePsi,
        Commands_AlarmAirFlow,
        Commands_AlarmPsi,
        Commands_AlarmO2Mix,
        Commands_AlarmRebreathing,
        Commands_SetZeroPsi,
        Commands_Count
    };

    const char* CommandsData[] = {
        "UNK",
        "STA",
        "ALI",
        "TRI",
        "FIO",
        "CUR",
        "INH",
        "EXH",
        "ALF",
        "ALP",
        "ALO",
        "ALR",
        "SZP",
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
    bool getValueArray(const uint8_t* pData, uint8_t& index, const uint8_t length, T*& value, uint8_t& count)
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

        if (index + (count * sizeof(T)) >= length)
        {
            return false;
        }

        if (count > 0)
        {
            value = new T[count];
            memset(value, 0, sizeof(T) * count);
            for (uint8_t n = 0; n < count; ++n)
            {
                if (!getValue<T>(pData, index, length, value[n]))
                {
                    delete[] value;
                    value = nullptr;
                    return false;
                }
            }
        }
        return true;
    }
}

bool SerialPortReader::ParseCommand(uint8_t* pData, uint8_t length)
{
    // replace the end of the string with 0s so it's compatible with the strcmp function
    uint8_t dataIndex = 0;

    // 3 first bytes is the command
    if (length < 3)
        return false;

    uint8_t command[4] = { pData[dataIndex++], pData[dataIndex++], pData[dataIndex++], '\0' };
    
    uint8_t commandIndex = 0;
    for (commandIndex = 1; commandIndex < Commands_Count; ++commandIndex)
    {
        // this reinterpret cast is annoying
        if (strcmp(reinterpret_cast<char*>(command), CommandsData[commandIndex]) == 0)
        {
            // we found our command!
            break;
        }
    }

    // TODO with more time we have to recheck this code which is super dependent on proper values.
    switch (commandIndex)
    {
    case Commands_Alive:
        Serial.print(ReturnCommands[ReturnCommands_ACK]);
        break;
    case Commands_Status:
    {
        char buffer[1024] = { 0 };
        sprintf_P(buffer, "PS1:%f,PS2:%f,RQP:%f,BAT:%f,PMP:%d,STA:%d,CTL:%d,TRG:%d,CYC:%d\r\n", 
            gDataModel.fPressure_mmH2O[0],
            gDataModel.fPressure_mmH2O[1],
            gDataModel.fRequestPressure_mmH2O,
            gDataModel.fBatteryLevel, 
            static_cast<int>(gDataModel.nPWMPump),
            static_cast<int>(gDataModel.nState),
            static_cast<int>(gDataModel.nControlMode),
            static_cast<int>(gDataModel.nTriggerMode),
            static_cast<int>(gDataModel.nCycleState));
        Serial.print(buffer);
    }
    break;
    case Commands_Trigger:
    {
        uint8_t temp = 0;
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
    case Commands_Fio:
    {
        uint8_t fio;
        if (getValue(pData, dataIndex, length, fio))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;
    case Commands_Curve:
    {
        float breatheRate = 0.0f;
        float* inhaleCurveMM = nullptr;
        float* exhaleCurveMM = nullptr;
        float* inhaleCurveTime = nullptr;
        float* exhaleCurveTime = nullptr;
        uint8_t inhaleCountMM = 0;
        uint8_t exhaleCountMM = 0;
        uint8_t inhaleCountTime = 0;
        uint8_t exhaleCountTime = 0;

        // breathe rate
        bool ok = getValue(pData, dataIndex, length, breatheRate);

        // Inhale
        if (ok)
        {
            ok = getValueArray(pData, dataIndex, length, inhaleCurveMM, inhaleCountMM);
        }
        if (ok)
        {
            ok = getValueArray(pData, dataIndex, length, inhaleCurveTime, inhaleCountTime);
        }

        // Exhale
        if (ok)
        {
            ok = getValueArray(pData, dataIndex, length, exhaleCurveMM, exhaleCountMM);
        }
        if (ok)
        {
            ok = getValueArray(pData, dataIndex, length, exhaleCurveTime, exhaleCountTime);
        }

        // TODO what should be the minimum amount of points in the curve?
        // Should we add warnings here?
        ok = inhaleCountMM == inhaleCountTime && exhaleCountMM == exhaleCountTime &&
             inhaleCountMM > 0 && exhaleCountMM > 0 && inhaleCountMM < kMaxCurveCount && exhaleCountMM < kMaxCurveCount;
        
        if (ok)
        {
            gDataModel.nRespirationPerMinute = static_cast<uint8_t>(breatheRate);

            gDataModel.pInhaleCurve.nCount = inhaleCountMM;
            for (uint8_t n = 0; n < inhaleCountMM; ++n)
            {
                gDataModel.pInhaleCurve.fSetPoint_mmH2O[n] = inhaleCurveMM[n];
                gDataModel.pInhaleCurve.nSetPoint_TickMs[n] = static_cast<uint32_t>(inhaleCurveTime[n] * (1.0f / gDataModel.nRespirationPerMinute) * 1000);
            }

            gDataModel.pExhaleCurve.nCount = exhaleCountMM;
            for (uint8_t n = 0; n < exhaleCountMM; ++n)
            {
                gDataModel.pExhaleCurve.fSetPoint_mmH2O[n] = exhaleCurveMM[n];
                gDataModel.pExhaleCurve.nSetPoint_TickMs[n] = static_cast<uint32_t>(exhaleCurveTime[n] * (1.0f / gDataModel.nRespirationPerMinute) * 1000);
            }
        }
        
        if (!ok)
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
    }
    break;
    case Commands_InhalePsi:
    {
        uint8_t inhalePsi;
        if (getValue(pData, dataIndex, length, inhalePsi))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;
    case Commands_ExhalePsi:
    {
        uint8_t exhalePsi;
        if (getValue(pData, dataIndex, length, exhalePsi))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;
    case Commands_AlarmAirFlow:
    {
        uint8_t* inhalePsi = nullptr;
        uint8_t count = 0;
        if (getValueArray(pData, dataIndex, length, inhalePsi, count))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
        delete[] inhalePsi;
        inhalePsi = nullptr;
    }
    break;
    case Commands_AlarmPsi:
    {
        uint8_t* psi = nullptr;
        uint8_t count = 0;
        if (getValueArray(pData, dataIndex, length, psi, count))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
        delete[] psi;
        psi = nullptr;
    }
    break;
    case Commands_AlarmO2Mix:
    {
        uint8_t* o2mix = nullptr;
        uint8_t count = 0;
        if (getValueArray(pData, dataIndex, length, o2mix, count))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
        delete[] o2mix;
        o2mix = nullptr;
    }
    break;
    case Commands_AlarmRebreathing:
    {
        uint8_t rebreathing;
        if (getValue(pData, dataIndex, length, rebreathing))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;
    case Commands_SetZeroPsi:
    {
        uint8_t zeropsi;
        if (getValue(pData, dataIndex, length, zeropsi))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
    }
    break;
    default:
        Serial.print(ReturnCommands[ReturnCommands_NACK]);
        break;
    }

    return true;
}