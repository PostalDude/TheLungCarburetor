///
/// \file       safeties.cpp
/// \brief      The Lung Carburetor Firmware safeties module
///
/// \author     Frederic Lauzon
/// \ingroup    safeties
#include "safeties.h"
#include "configuration.h"
#include "datamodel.h"

tSafeties gSafeties;

// Initialize safeties
bool Safeties_Init()
{
    gSafeties.bEnabled              = true;
    gSafeties.bCritical             = false;
    gSafeties.bConfigurationInvalid = false;

    return true;
}

void Safeties_Clear()
{
    gSafeties.bCritical             = false;
    gDataModel.nSafetyFlags         = 0;
}

bool Safeties_Enable()
{
    gSafeties.bEnabled = true;
    return true;
}

bool Safeties_Disable()
{
    gSafeties.bEnabled = false;
    return true;
}

// Process safeties checks
void Safeties_Process()
{
    if (gDataModel.nState != kState_Process)
    {
        return;
    }

    // If any safety issue, set bCritical in global safeties structure
    if (gSafeties.bEnabled)
    {
        float fPressureDelta = gDataModel.fPressure_mmH2O[0] - gDataModel.fPressure_mmH2O[1];

        gDataModel.nSafetyFlags = 0;
        if (gDataModel.fPressure_mmH2O[0] >= gConfiguration.fMaxPressureLimit_mmH2O)
        {
            gDataModel.nSafetyFlags |= kAlarm_MaxPressureLimit;
        }

        if (gDataModel.fPressure_mmH2O[1] <= gConfiguration.fMinPressureLimit_mmH2O)
        {
            gDataModel.nSafetyFlags |= kAlarm_MinPressureLimit;
        }

        if (fabs(fPressureDelta) >= gConfiguration.fMaxPressureDelta_mmH2O)
        {
            gDataModel.nSafetyFlags |= kAlarm_PressureSensorRedudancyFail;
        }

        if (gSafeties.bConfigurationInvalid)
        {
            gDataModel.nSafetyFlags |= kAlarm_InvalidConfiguration;
        }

        if (gDataModel.fBatteryLevel < gConfiguration.fMinBatteryLevel)
        {
            gDataModel.nSafetyFlags |= kAlarm_BatteryLow;
        }

        if (gDataModel.nSafetyFlags != 0)
        {
            gSafeties.bCritical     = true;
            gDataModel.nState       = kState_Error;
        }
    }
}
