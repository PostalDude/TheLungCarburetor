#include "configuration.h"

tConfiguration gConfiguration;

// Read configuration from eeprom
bool Configuration_Init()
{
	memset(&gConfiguration, 0, sizeof(tConfiguration));
	return Configuration_Read();
}

bool Configuration_Read()
{
	//*** Read eeprom
	//*** Check CRC
	
	return true;
}

bool Configuration_Write()
{
	return true;
}

