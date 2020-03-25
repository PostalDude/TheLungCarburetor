#ifndef TLC_SAFETIES_H
#define TLC_SAFETIES_H

#include "common.h"

struct tSafeties
{
	bool	bEnabled;				// Safeties are enabled
	bool	bCritical;				// Critical safety, details in this structure
	bool	bConfigurationInvalid;	// Configuration is invalid
};
extern tSafeties gSafeties;

bool Safeties_Init();
void Safeties_Clear();
bool Safeties_Enable();
bool Safeties_Disable();
void Safeties_Process();

#endif // TLC_SAFETIES_H