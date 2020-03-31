///
/// \file 		gpio.h
/// \brief 		The Lung Carburetor Firmware gpio module
///				
/// \author 	Frederic Lauzon
/// \defgroup 	gpio GPIO
#ifndef TLC_GPIO_H
#define TLC_GPIO_H

#include "common.h"

///	\fn bool GPIO_Init()
/// \brief Initialize pin directions
bool GPIO_Init();

#endif // TLC_GPIO_H