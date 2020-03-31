///
/// \file 		sensors.h
/// \brief 		The Lung Carburetor Firmware sensors module
///				
/// \author 	Frederic Lauzon
/// \defgroup 	sensors Sensors
#ifndef TLC_SENSORS_H
#define TLC_SENSORS_H

#include "common.h"

///	\fn bool Sensors_Init()
/// \brief Initialize sensors
bool Sensors_Init();

///	\fn bool Sensors_Process()
/// \brief Process sensors readings
void Sensors_Process();

#endif // TLC_SENSORS_H