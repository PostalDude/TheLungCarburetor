#include "safeties.h"
#include "configuration.h"
#include "datamodel.h"

tSafeties gSafeties;

// Initialize safeties
bool Safeties_Init()
{
	gSafeties.bEnabled 				= true;
	gSafeties.bCritical				= false;
	gSafeties.bConfigurationInvalid	= false;

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
	if (gSafeties.bEnabled)
	{
		if (gSafeties.bConfigurationInvalid ||			
			((gDataModel.nPressureLimits & kPressureLimit_High) != 0) ||
			((gDataModel.nPressureLimits & kPressureLimit_Low)  != 0))
		{
			gSafeties.bCritical = true;
			gDataModel.nState = kState_Error;
		}
	}
}
