///
/// \file 		serialportreader.h
/// \brief 		The Lung Carburetor Firmware serialportreader module
///				
///	A serial port protocol parser
///
/// \author 	Adam Galarneau
/// \defgroup 	serialportreader Serial Port Reader
#pragma once

#include <stdint.h>

///	\fn bool UpdateCurve()
/// \brief Update curve
bool UpdateCurve();

///	\fn bool ParseCommand()
/// \brief Parse command receive from serial port
bool ParseCommand(uint8_t* pData, uint8_t length);
