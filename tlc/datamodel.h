#ifndef TLC_DATAMODEL_H
#define TLC_DATAMODEL_H

#include "common.h"

// Data Model structure, filled by sensors
struct tDataModel
{
	eState		nState;					// System state
	uint8_t		nRawPressure;			// Raw read pressure from sensor
	float 		fRequestPressure_cmH2O;	// Requested pressure set-point
	float 		fPressure_cmH2O;		// Converted pressure, useable as cmH2O
	float 		fPressureError;			// Pressure error: readings vs set-point
	float		fP;						// Control Proportional
	float		fI;						// Control Integral
	float		fD;						// Control Derivative
	float		fPI;					// Control Sum of Proportional and Integral errors
	uint8_t		nPressureLimits;		// Bitmaks of ePressureLimit	
	uint8_t		nPWMPump;				// Pump PWM power output
	uint32_t	nTickControl;			// Last control tick
	uint32_t	nTickCommunications;	// Last communications tick
	uint32_t	nTickSensors;
};

extern tDataModel gDataModel;

bool DataModel_Init();
void DataModel_Process();

#endif // TLC_DATAMODEL_H
