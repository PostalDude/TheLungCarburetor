#ifndef TLC_DATAMODEL_H
#define TLC_DATAMODEL_H

#include "common.h"

// Describe a pressurecurve to execute
struct tPressureCurve
{
    float           fSetPoint_mmH2O[kMaxCurveCount];    // Pressure for every point of the curve
    uint32_t        nSetPoint_TickMs[kMaxCurveCount];   // Number of millisecond to execute point of the curve
    uint8_t         nCount;                             // Number of active points in the setpoint curve
};

// Data Model structure, filled by sensors
struct tDataModel
{
    eState          nState;                 // System state
    eControlMode    nControlMode;           // Control mode of the pump
    eTriggerMode    nTriggerMode;           // Respiration trigger mode
    uint8_t         nRawPressure[2];        // Raw read pressure from sensor
    float           fBatteryLevel;          // Battery voltage level

    eCycleState     nCycleState;            // Respiration cycle state
    tPressureCurve  pInhaleCurve;           // Inhale curve descriptor
    tPressureCurve  pExhaleCurve;           // Exhale curve descriptor
    uint8_t         nCurveIndex;            // Current executing curve setpoint index

    float           fRequestPressure_mmH2O; // Requested pressure set-point
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
