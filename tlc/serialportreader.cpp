#include "serialportreader.h"

#include "common.h"

namespace
{
    
    template <typename T>
    static bool getValue(const uint8_t* pData, uint8_t& index, const uint8_t length, T& value)
    {
        if (index >= length)
        {
            return false;
        }
        else
        {
            value = (T)pData[index];
            index += sizeof(T);
            return true;
        }
    }

    template <typename T>
    static bool getValueArray(const uint8_t* pData, uint8_t& index, const uint8_t length, T*& value)
    {
        value = nullptr;

        if (index >= length)
        {
            return false;
        }

        uint8_t count = pData[index++];
        if (index + count >= length)
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

    enum Commands
    {
        Commands_Unknown = 0,
        Commands_Status,
        Commands_Alive,
        Commands_Count
    };

    const char* CommandsData[] = {
        "UNK",
        "STA",
        "ALI"
    };

    enum ReturnCommands
    {
        ReturnCommands_ACK,
        ReturnCommands_NACK,
        ReturnCommands_Count
    };

    const char* ReturnCommands[] = {
        "NACK\r\n",
        "ACK\r\n",
        "NACK\r\n"
    };

}

bool SerialPortReader::ParseCommand(uint8_t* pData, uint8_t length)
{
    // replace the end of the string with 0s so it's compatible with the strcmp function
    if (pData[length - 2] != '\r' || pData[length - 1] != '\n')
    {
        return false;
    }
    pData[length - 2] = '\0';
    pData[length - 1] = '\0';

    uint8_t commandIndex = 0;
    for (commandIndex = 1; commandIndex < Commands_Count; ++commandIndex)
    {
        if (strcmp(reinterpret_cast<const char*>(pData), CommandsData[commandIndex]) == 0)
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
        Serial.print(ReturnCommands[ReturnCommands_NACK]);
        break;
    default:
        Serial.print(ReturnCommands[ReturnCommands_NACK]);
        break;
    }

    return true;
}