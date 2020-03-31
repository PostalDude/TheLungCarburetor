///
/// \file       configuration.h
/// \brief      The Lung Carburetor Firmware configuration module
///
/// \author     Frederic Lauzon
/// \defgroup   configuration Configuration
#ifndef TLC_CONFIGURATION_H
#define TLC_CONFIGURATION_H

#include "common.h"

/// \struct tConfiguration
/// \brief NVM Configuration Stored and Loaded from EEPROM.
struct tConfiguration
{
    uint8_t     nVersion;                   ///> Configuration structure version
    uint16_t    nPressureSensorOffset[2];   ///> Offset when pressure sensor is at atmosphere readings
    float       fMinBatteryLevel;           ///> Minimum battery level for alarm
    float       fMaxPressureLimit_mmH2O;    ///> Max allowed pressure limit
    float       fMinPressureLimit_mmH2O;    ///> Min allowed pressure limit
    float       fMaxPressureDelta_mmH2O;    ///> Maximum allowed pressure delta between redundant readings
    float       fGainP;                     ///> Control gain P
    float       fGainI;                     ///> Control gain I
    float       fGainD;                     ///> Control gain D
    float       fILimit;                    ///> Integral error limit
    float       fPILimit;                   ///> Proportional+Integral error limit
    float       fControlTransfer;           ///> Control transfer from adjusted errors to pwm
    float       fPatientTrigger_mmH2O;      ///> Patient triggers respiration when this value is reached (In TriggerMode Patient or semi automatic)
    uint16_t    nServoExhaleOpenAngle;      ///> Angle in degree (0..180) when exhale servo valve is open
    uint16_t    nServoExhaleCloseAngle;     ///> Angle in degree (0..180) when exhale servo valve is close
    uint32_t    nCRC;                       ///> Configuration CRC check
};
extern tConfiguration gConfiguration;

// Make sure that configuration structure can fit into the EEPROM
HXCOMPILATIONASSERT(assertEEPROMSizeCheck, (sizeof(tConfiguration) <= 512));

/// \fn bool Configuration_Init()
/// \brief Initialize configuration module
bool Configuration_Init();

/// \fn bool Configuration_SetDefaults()
/// \brief Set default configuration
bool Configuration_SetDefaults();

/// \fn bool Configuration_Read()
/// \brief Read configuration from eeprom
bool Configuration_Read();

/// \fn bool Configuration_Write()
/// \brief Write configuration to eeprom
bool Configuration_Write();

#endif // TLC_CONFIGURATION_H