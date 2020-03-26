#ifndef TLC_CONFIGURATION_H
#define TLC_CONFIGURATION_H

#include "common.h"

// NVM Configuration Stored and Loaded from EEPROM. We keep native alignment.
struct tConfiguration
{
    uint8_t     nVersion;                   // Configuration structure version
    uint16_t    nPressureSensorOffset[0];   // Offset when pressure sensor is at atmosphere readings    
    float       fMaxPressureLimit_mmH2O;    // Max allowed pressure limit
    float       fMinPressureLimit_mmH2O;    // Min allowed pressure limit
    float       fMaxPressureDelta_mmH2O;    // Maximum allowed pressure delta between redundant readings
    float       fGainP;                     // Control gain P
    float       fGainI;                     // Control gain I
    float       fGainD;                     // Control gain D
    float       fILimit;                    // Integral error limit
    float       fPILimit;                   // Proportional+Integral error limit
    float       fControlTransfer;           // Control transfer from adjusted errors to pwm
    uint32_t    nCRC;                       // Configuration CRC check
};
extern tConfiguration gConfiguration;

// Make sure that configuration structure can fit into the EEPROM
HXCOMPILATIONASSERT(assertEEPROMSizeCheck, (sizeof(tConfiguration) <= 512));

bool Configuration_Init();
bool Configuration_Read();
bool Configuration_Write();

#endif // TLC_CONFIGURATION_H