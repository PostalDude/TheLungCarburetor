#ifndef TLC_DATAMODEL_H
#define TLC_DATAMODEL_H

#include "common.h"

// Data Model structure, filled by sensors
struct tDataModel
{
    eState          nState;                 // System state
    eControlMode    nControlMode;           // Control mode of the pump
	eTriggerMode	nTriggerMode;			// Respiration trigger mode
    uint8_t         nRawPressure[2];        // Raw read pressure from sensor
	float			fBatteryLevel;			// Battery voltage level
        
    float           fRequestPressure_mmH2O; // Requested pressure set-point
    uint8_t         nRespirationCurveIndex; // Current index in the respiration curve
    uint8_t         nRespirationCurveCount; // Number of index in the respiration curve
    float           fRespirationCurve_mmH2O[kMaxRespirationCurveCount];
    uint32_t        nRespirationCurve_Tick[kMaxRespirationCurveCount];
    uint8_t         nRespirationPerMinute;  // Number of respiration per minute
    
    float           fPressure_mmH2O[2];     // Converted pressure, useable as cmH2O
    float           fPressureError;         // Pressure error: readings vs set-point
    float           fP;                     // Control Proportional
    float           fI;                     // Control Integral
    float           fD;                     // Control Derivative
    float           fPI;                    // Control Sum of Proportional and Integral errors
    uint8_t         nPWMPump;               // Pump PWM power output
    
    uint32_t        nTickControl;           // Last control tick
    uint32_t        nTickCommunications;    // Last communications tick
    uint32_t        nTickSensors;           // Last sensors tick
    uint32_t        nTickSetPoint;          // Current curve pressure set-point ticker
    uint32_t        nTickRespiration;       // Start of respiration tick
};

extern tDataModel gDataModel;

bool DataModel_Init();
void DataModel_Process();

#endif // TLC_DATAMODEL_H
