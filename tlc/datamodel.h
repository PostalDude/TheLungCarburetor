///
/// \file       datamodel.h
/// \brief      The Lung Carburetor Firmware datamodel definitions file
///
/// \author     Frederic Lauzon
/// \defgroup   datamodel Data Model
#ifndef TLC_DATAMODEL_H
#define TLC_DATAMODEL_H

#include "common.h"

/// \struct tPressureCurve
/// \brief Describe a pressurecurve to execute
///
/// A collection of setpoints of pressure in time to execute for the respiration cycle
struct tPressureCurve
{
    float           fSetPoint_mmH2O[kMaxCurveCount];    ///> Pressure for every point of the curve
    uint32_t        nSetPoint_TickMs[kMaxCurveCount];   ///> Number of millisecond to execute point of the curve
    uint8_t         nCount;                             ///> Number of active points in the setpoint curve
};

/// \struct tDataModel
/// \brief Describe internal data
///
/// A centralised collection of information read and written by modules
struct tDataModel
{
    uint16_t        nSafetyFlags;           ///> Bitmask of safety alarm source eAlarm
    bool            bStartFlag;             ///> If system is started, start respiration cycle
    eState          nState;                 ///> System state
    eControlMode    nControlMode;           ///> Control mode of the pump
    eTriggerMode    nTriggerMode;           ///> Respiration trigger mode
    uint8_t         nRawPressure[2];        ///> Raw read pressure from sensor
    float           fBatteryLevel;          ///> Battery voltage level

    eCycleState     nCycleState;            ///> Respiration cycle state
    tPressureCurve  pInhaleCurve;           ///> Inhale curve descriptor
    tPressureCurve  pExhaleCurve;           ///> Exhale curve descriptor
    uint8_t         nCurveIndex;            ///> Current executing curve setpoint index

    float           fRequestPressure_mmH2O; ///> Requested pressure set-point
    uint8_t         nRespirationPerMinute;  ///> Number of respiration per minute
    float           fInhalePressureTarget_mmH2O; ///> Inhale Pressure Target
    float           fExhalePressureTarget_mmH2O; ///> Exhale Pressure Target
    float           fInhaleRatio;           ///> Inhale Ratio
    float           fExhaleRatio;           ///> Exhale Ratio

    float           fPressure_mmH2O[2];     ///> Converted pressure, useable as cmH2O
    float           fPressureError;         ///> Pressure error: readings vs set-point
    float           fP;                     ///> Control Proportional
    float           fI;                     ///> Control Integral
    float           fD;                     ///> Control Derivative
    float           fPI;                    ///> Control Sum of Proportional and Integral errors
    float           fPWMPump;               ///> Pump PWM power output

    uint32_t        nTickControl;           ///> Last control tick
    uint32_t        nTickCommunications;    ///> Last communications tick
    uint32_t        nTickSensors;           ///> Last sensors tick
    uint32_t        nTickSetPoint;          ///> Current curve pressure set-point ticker
    uint32_t        nTickRespiration;       ///> Start of respiration tick
    uint32_t        nTickStabilization;     ///> Stabilization tick between respiration
    uint32_t        nTickWait;              ///> Wait tick after respiration
    uint32_t        nTickLcdKeypad;         ///> Lcd and Keypad update and scan rate
};

extern tDataModel gDataModel;

/// \fn bool DataModel_Init()
/// \brief Initialize datamodel defaults
bool DataModel_Init();

#endif // TLC_DATAMODEL_H
