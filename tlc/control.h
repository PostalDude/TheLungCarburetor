///
/// \file       control.h
/// \brief      The Lung Carburetor Firmware control module
///
/// \author     Frederic Lauzon
/// \defgroup   control Respiration control
#ifndef TLC_CONTROL_H
#define TLC_CONTROL_H

#include "common.h"
#include <ServoTimer2.h>
extern ServoTimer2 exhaleValveServo;

/// \fn bool Control_Init()
/// \brief Initialize control
bool Control_Init();

/// \fn bool Control_Process()
/// \brief Process control
void Control_Process();

#endif // TLC_CONTROL_H
