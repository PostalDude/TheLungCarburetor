///
/// \file 		defs.h
/// \brief 		The Lung Carburetor Firmware definitions file
///				
/// \author 	Frederic Lauzon
#ifndef TLC_DEFS_H
#define TLC_DEFS_H

// Force error check at compile time for constants
#define HXCOMPILATIONASSERT(name, x) typedef char name[x ? 1 : -1]

///	\enum eConsts
/// \brief General constants used throughout the firmware
enum eConsts
{
    kSerialBaudRate             = 115200,   ///> Baud rate of serial port
    kSerialRxTimeOut            = 10,       ///> Maximum wait time in ms to wait for serial data
    kRxBufferSize               = 250,      ///> Maximum rx buffer size
    kRxBufferReserve            = 10,       ///> Reserve of data before we start discarding rx buffer
    kSerialDiscardTimeout       = 500,      ///> Discard rx buffer timeout
    kPeriodCommPublish          = 500,      ///> Period to send status information to controller
    kPeriodControl              = 5,        ///> Period to call control loop in milliseconds
    kPeriodCommunications       = 2,        ///> Period to call communications loop in milliseconds
    kPeriodLcdKeypad            = 250,      ///> Period to refresh Lcd and scan keypad in milliseconds
    kPeriodSensors              = 5,        ///> Period to call sensors loop in milliseconds
    kPeriodWarmup               = 1000,     ///> Period to warmup the system in milliseconds
    kPeriodStabilization        = 100,      ///> Stablization period between respiration cycles
    kEEPROM_Version             = 1,        ///> EEPROM version must match this version for compatibility
    kMaxCurveCount              = 8,       ///> Maximum respiration curve index count
};

HXCOMPILATIONASSERT(assertSensorPeriodCheck, (kPeriodSensors >= 1));
HXCOMPILATIONASSERT(assertRXBufferSizeCheck, (kRxBufferSize < 255));

///	\enum eState
/// \brief System state
enum eState
{
    kState_Init = 0,	///> System initializing
    kState_Idle,		///> System is idle
    kState_Warmup,		///> System is warming up and sampling sensors
    kState_Process,		///> System is processing respiration and sensors
    kState_Error,		///> System raised an error

    kState_Count
};

///	\enum eState
/// \brief Respiration Cycle State
enum eCycleState
{
    kCycleState_WaitTrigger = 0,	///> Respiration cycle waiting for trigger
    kCycleState_Inhale,				///> Respiration cycle inhaling
    kCycleState_Exhale,				///> Respiration cycle exnhaling
    kCycleState_Stabilization,		///> Respiration cycle stabilization of pressure

    kCycleState_Count
};

///	\enum eControlMode
/// \brief Pump pressure control mode
enum eControlMode
{
    kControlMode_PID = 0,       ///> Pump is controlled by pressure feedback using a PID
    kControlMode_FeedForward,   ///> Feedforward is used to send requested pump pwm values from master controller

    kControlMode_Count
};

///	\enum eTriggerMode
/// \brief Respiration trigger mode
enum eTriggerMode
{
    kTriggerMode_Timed = 0,             ///> Machine Respiration timed, ignore patient respiration
    kTriggerMode_Patient,               ///> Machine Respiration triggered by patient respiration
    kTriggerMode_PatientSemiAutomatic,  ///> Machine Respiration triggered by patient respiration, or timed when patient is not triggering after a timeout

    kTriggerMode_Count
};

///	\enum eAlarm
/// \brief Alarms raised by safeties
enum eAlarm
{
    kAlarm_MaxPressureLimit             = (1<<0),	///> System max pressure reached
    kAlarm_MinPressureLimit             = (1<<1),	///> System min pressure reached
    kAlarm_PressureSensorRedudancyFail  = (1<<2),	///> Both pressure sensors report different readings
    kAlarm_InvalidConfiguration         = (1<<3), 	///> Loaded configuration is invalid
    kAlarm_BatteryLow                   = (1<<4),	///> Low battery voltage
};

const float kMPX5010_MaxPressure_mmH2O          = 1019.78f;
const float kMPX5010_MaxPressureDelta_mmH2O     = 40.0f;
const float kMPX5010_Accuracy                   = 0.5f;
const float kMPX5010_Sensitivity_mV_mmH2O       = 4.413f;
const float kBatteryLevelGain                   = 3.0f;

#define PIN_SERIAL_RX           0       // Serial port RX
#define PIN_SERIAL_TX           1       // Serial port TX

#define PIN_OUT_SERVO_EXHALE    2       // Servo exhale valve

// Timer0 used by millis
// timer1 used by TimerOne
// Timer2 used by ServoTimer2
// pins 3 and 11 analogWrite are disabled by the use of servotimer2 library

// this pin cannot be changes since we use the timer1 port
#define PIN_OUT_PUMP1_PWM       9       // Ambu pump Cam PWM output

#define PIN_OUT_BUZZER          5      // Buzzer signal output

#define PIN_PRESSURE0           A0      // Pressure readings from MPX pressure sensor
#define PIN_PRESSURE1           A1      // Pressure readings from MPX redundant pressure sensor
#define PIN_BATTERY             A2      // Battery voltage

#define PIN_LCD_KEYPAD_SDA      A4
#define PIN_LCD_KEYPAD_SCL      A5

#endif // TLC_DEFS_H
