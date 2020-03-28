#include "serialportreader.h"
#include "common.h"
#include "datamodel.h"

#include <sstream>

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

    char command[4] = { pData[dataIndex++], pData[dataIndex++], pData[dataIndex++], '\0' };

    uint8_t commandIndex = 0;
    for (commandIndex = 1; commandIndex < Commands_Count; ++commandIndex)
    {
        if (strcmp(command, CommandsData[commandIndex]) == 0)
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
        std::stringstream ss;
        ss << "PS1:" << gDataModel.fPressure_mmH2O[0]
            << ",PS2:" << gDataModel.fPressure_mmH2O[1]
            << ",RQP:" << gDataModel.fRequestPressure_mmH2O
            << ",BAT:" << gDataModel.fBatteryLevel
            << ",PMP:" << (int)gDataModel.nPWMPump
            << ",STA:" << (int)gDataModel.nState
            << ",CTL:" << (int)gDataModel.nControlMode
            << ",TRG:" << (int)gDataModel.nTriggerMode
            << ",CYC:" << (int)gDataModel.nCycleState
            << "\r\n";
        Serial.print(ss.str().c_str());
    }
    break;
    case Commands_Trigger:
    {
        float* temp = 0;
        uint8_t count;
        if (getValueArray(pData, dataIndex, length, temp, count))
            Serial.print(ReturnCommands[ReturnCommands_ACK]);
        else
            Serial.print(ReturnCommands[ReturnCommands_NACK]);
        delete[] temp;
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
        Serial.print(ReturnCommands[ReturnCommands_NACK]);
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