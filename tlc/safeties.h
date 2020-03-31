///
/// \file       safeties.h
/// \brief      The Lung Carburetor Firmware safeties module
///
/// \author     Frederic Lauzon
/// \defgroup   safeties Safeties
#ifndef TLC_SAFETIES_H
#define TLC_SAFETIES_H

#include "common.h"

/// \struct tSafeties
/// \brief Safeties descriptor
struct tSafeties
{
    bool    bEnabled;               ///> Safeties are enabled
    bool    bCritical;              ///> Critical safety, details in this structure
    bool    bConfigurationInvalid;  ///> Configuration is invalid
};
extern tSafeties gSafeties;

/// \fn bool Safeties_Init()
/// \brief Initialize safeties module

/// \fn bool Safeties_Clear()
/// \brief Clear alarms
void Safeties_Clear();

/// \fn bool Safeties_Enable()
/// \brief Enable safeties
bool Safeties_Enable();

/// \fn bool Safeties_Disable()
/// \brief Disable safeties
bool Safeties_Disable();

/// \fn bool Safeties_Process()
/// \brief Process safeties and raise alarms if needed
void Safeties_Process();

#endif // TLC_SAFETIES_H
