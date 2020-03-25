#include "communications.h"
#include "datamodel.h"

static bool gSerialConnected = false;

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
		}
	}
}
