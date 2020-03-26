#include "sensors.h"
#include "datamodel.h"
#include "configuration.h"

// Initialize sensor devices
bool Sensors_Init()
{
    return true;
}

// Process sensors sampling
void Sensors_Process()
{
    if (gDataModel.nState != kState_Process || gDataModel.nState != kState_Warmup)
    {
        return;
    }

    gDataModel.nRawPressure[0] = analogRead(PIN_PRESSURE0) - gConfiguration.nPressureSensorOffset[0];
    gDataModel.nRawPressure[1] = analogRead(PIN_PRESSURE1) - gConfiguration.nPressureSensorOffset[1];
    
    // Transfer function for raw pressure.
    float mV    = (float)gDataModel.nRawPressure[0] * (1.0f/1024.0f) * 5000.0f; // Voltage in millivolt measured on ADC
    float mmH2O = mV * (1.0f / kMPX5010_Sensitivity_mV_mmH2O);
    
    gDataModel.fPressure_mmH2O[0] = mmH2O;
    
    // Redundant pressure reading, for safeties
    mV      = (float)gDataModel.nRawPressure[1] * (1.0f/1024.0f) * 5000.0f; // Voltage in millivolt measured on ADC
    mmH2O   = mV * (1.0f / kMPX5010_Sensitivity_mV_mmH2O);
    
    gDataModel.fPressure_mmH2O[1] = mmH2O;  
}
