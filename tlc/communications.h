///
/// \file 		communications.h
/// \brief 		The Lung Carburetor Firmware communications module
///			
///	The communications module parse commands coming from the serial port
///	
/// \author 	Frederic Lauzon
/// \defgroup 	communications Communications
#ifndef TLC_COMMUNICATIONS_H
#define TLC_COMMUNICATIONS_H

///	\fn bool Communications_Init()
/// \brief Initialize communications module
bool Communications_Init();

///	\fn bool Communications_Process()
/// \brief Process communications
void Communications_Process();

#endif // TLC_COMMUNICATIONS_H
