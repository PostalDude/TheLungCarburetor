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
    gSafeties.bCritical = false;
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
 #if 0 //*** Put me back in normal operation to enable safeties
    if (gSafeties.bEnabled)
    {
        float fPressureDelta = gDataModel.fPressure_mmH2O[0] - gDataModel.fPressure_mmH2O[1];

        if (gSafeties.bConfigurationInvalid                                         ||
            gSafeties.bCritical                                                     ||
            gDataModel.fPressure_mmH2O[0] >= gConfiguration.fMaxPressureLimit_mmH2O ||
            gDataModel.fPressure_mmH2O[1] <= gConfiguration.fMinPressureLimit_mmH2O ||
            fabs(fPressureDelta)          >= gConfiguration.fMaxPressureDelta_mmH2O)
        {
            gSafeties.bCritical = true;
            gDataModel.nState   = kState_Error;
        }
    }
#endif

}
