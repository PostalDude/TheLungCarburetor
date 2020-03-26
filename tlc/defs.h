#ifndef TLC_DEFS_H
#define TLC_DEFS_H

// Force error check at compile time for constants
#define HXCOMPILATIONASSERT(name, x) typedef char name[x ? 1 : -1]


// This file contain values shared with an external control on the serial port
enum eConsts
{
    kSerialBaudRate             = 115200,   // Baud rate of serial port
    kPeriodControl              = 5,        // Period to call control loop in milliseconds
    kPeriodCommunications       = 2,        // Period to call communications loop in milliseconds
    kPeriodSensors              = 5,        // Period to call sensors loop in milliseconds
    kPeriodWarmup               = 1000,     // Period to warmup the system in milliseconds
    kEEPROM_Version             = 1,        // EEPROM version must match this version for compatibility
    kMaxCurveCount              = 64,       // Maximum respiration curve index count
};

HXCOMPILATIONASSERT(assertSensorPeriodCheck, (kPeriodSensors >= 1));

// State of the system
enum eState
{
    kState_Idle = 0,
    kState_Init,
    kState_Warmup,
    kState_Process,
    kState_Error,

    kState_Count
};

// Respiration Cycle State
enum eCycleState
{
    kCycleState_WaitTrigger = 0,
    kCycleState_Inhale,
    kCycleState_Exhale,

    kCycleState_Count
};

// Pump Servo Control Mode
enum eControlMode
{
    kControlMode_PID = 0,       // Pump is controlled by pressure feedback using a PID
    kControlMode_FeedForward,   // Feedforward is used to send requested pump pwm values from master controller

    kControlMode_Count
};

// Respiration trigger mode
enum eTriggerMode
{
    kTriggerMode_Timed = 0,             // Machine Respiration timed, ignore patient respiration
    kTriggerMode_Patient,               // Machine Respiration triggered by patient respiration
    kTriggerMode_PatientSemiAutomatic,  // Machine Respiration triggered by patient respiration, or timed when patient is not triggering after a timeout

    kTriggerMode_Count
};

const float kMPX5010_MaxPressure_mmH2O          = 1019.78f;
const float kMPX5010_Accuracy                   = 0.5f;
const float kMPX5010_Sensitivity_mV_mmH2O       = 4.413f;

#define PIN_SERIAL_RX           0       // Serial port RX
#define PIN_SERIAL_TX           1       // Serial port TX

#define PIN_SERVO_EXHALE        2       // Servo exhale valve

#define PIN_OUT_LED             7       // LED debug output

#define PIN_OUT_PWM_PUMP        5       // Ambu pump Cam PWM output
#define PIN_OUT_PWM_BUZZER      9       // Buzzer PWM signal output

#define PIN_PRESSURE0           A0      // Pressure readings from MPX pressure sensor
#define PIN_PRESSURE1           A1      // Pressure readings from MPX redundant pressure sensor
#define PIN_BATTERY             A2      // Battery voltage

#endif // TLC_DEFS_H
