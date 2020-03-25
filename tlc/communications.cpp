#include "communications.h"
#include "datamodel.h"

static bool gSerialConnected = false;
static char rxBuffer[64];
static int rxIndex = 0;

bool Communications_Init()
{
	gSerialConnected = false;
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
			gSerialConnected = true;
		}
	}
	else
	{
		// Process input string, wait for AT .... <cr><lf>
		if (Serial.available() > 0)
		{
			char receivedByte = Serial.read();

			switch (receivedByte)
			{
			case '\n':
				char output[255];
				sprintf(output, "command received: %s", rxBuffer);
				Serial.print(output);
				rxIndex = 0;
				break;
			default:
				rxBuffer[rxIndex] = receivedByte;
				break;
			}
			rxIndex++;
		}
	}
}
