#include "sensors.h"
#include "datamodel.h"
#include "configuration.h"
#include "lcd_keypad.h"

// Initialize sensor devices
bool Sensors_Init()
{
    return true;
}

static int gSetZero = 100;

// Process sensors sampling
void Sensors_Process()
{
    if (!(gDataModel.nState == kState_Process || gDataModel.nState == kState_Warmup))
    {
      return;
    }


    gDataModel.nRawPressure[0] = analogRead(PIN_PRESSURE0);
    gDataModel.nRawPressure[1] = analogRead(PIN_PRESSURE1);

    if (gDataModel.nRawPressure[0] > gConfiguration.nPressureSensorOffset[0])
    {
      gDataModel.nRawPressure[0] -= gConfiguration.nPressureSensorOffset[0];
    }
    else
    {
      gDataModel.nRawPressure[0] = 0;
    }

    if (gDataModel.nRawPressure[1] > gConfiguration.nPressureSensorOffset[1])
    {
      gDataModel.nRawPressure[1] -= gConfiguration.nPressureSensorOffset[1];
    }
    else
    {
      gDataModel.nRawPressure[1] = 0;
    }

    //*** Hack for setzero at boot temporarily, WILL BE REMOVED!
#if 1
    if (gSetZero > 0)
    {
        --gSetZero;
        gConfiguration.nPressureSensorOffset[0] = analogRead(PIN_PRESSURE0);
    }
#endif

    // Transfer function for raw pressure.
    float mV    = (float)gDataModel.nRawPressure[0] * (1.0f/1024.0f) * 5000.0f; // Voltage in millivolt measured on ADC
    float mmH2O = mV * (1.0f / kMPX5010_Sensitivity_mV_mmH2O);

    gDataModel.fPressure_mmH2O[0] = mmH2O;

    // Redundant pressure reading, for safeties
    mV      = (float)gDataModel.nRawPressure[1] * (1.0f/1024.0f) * 5000.0f; // Voltage in millivolt measured on ADC
    mmH2O   = mV * (1.0f / kMPX5010_Sensitivity_mV_mmH2O);

    gDataModel.fPressure_mmH2O[1] = mmH2O;


    char szPressure[6];
    // 4 is mininum width, 2 is precision; float value is copied onto str_temp
    dtostrf(gDataModel.fPressure_mmH2O[0], 4, 2, szPressure);
    sprintf(gLcdMsg,"mmH2O:%s", szPressure);

    gDataModel.fBatteryLevel = (float)analogRead(PIN_BATTERY) * (1.0f/1024.0f) * (kBatteryLevelGain * 5.0f);
}
