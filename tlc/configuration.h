#ifndef TLC_CONFIGURATION_H
#define TLC_CONFIGURATION_H

#include "common.h"

// NVM Configuration Stored and Loaded from EEPROM
struct tConfiguration
{
	uint8_t		nVersion;				// Configuration structure version
	uint16_t	nPressureSensorOffset;	// Offset when pressure sensor is at atmosphere readings	
	float		fMaxPressureLimit;		// Max allowed pressure limit
	float		fMinPressureLimit;		// Min allowed pressure limit
	float		fGainP;					// Control gain P
	float		fGainI;					// Control gain I
	float		fGainD;					// Control gain D
	float		fILimit;				// Integral error limit
	float		fPILimit;				// Proportional+Integral error limit
	float		fControlTransfer;		// Control transfer from adjusted errors to pwm
	uint32_t	nCRC;					// Configuration CRC check
};
extern tConfiguration gConfiguration;

bool Configuration_Init();
bool Configuration_Read();
bool Configuration_Write();

#endif // TLC_CONFIGURATION_H