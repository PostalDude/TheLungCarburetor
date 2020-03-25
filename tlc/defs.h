#ifndef TLC_DEFS_H
#define TLC_DEFS_H

// This file contain values shared with an external control on the serial port

enum eConsts
{
	kSerialBaudRate 		= 115200,	// Baud rate of serial port
	kPeriodControl		= 5,		// Period to call control loop in milliseconds
	kPeriodCommunications	= 2,		// Period to call communications loop in milliseconds
	kPeriodSensors			= 10,		// Period to call sensors loop in milliseconds
	kPeriodWarmup			= 1000,		// Period to warmup the system in milliseconds	
};

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

// Pressure Limit on the manometer, read by reed switches
enum ePressureLimit
{
	kPressureLimit_High	= (1<<0),
	kPressureLimit_Mid 	= (1<<1),
	kPressureLimit_Low 	= (1<<2),
};

const float kMPX53DP_MaxPressureKPA 			= 50.0f;
const float kMPX53DP_SensitivityVoltPerKPA 		= 1.2f;
const float kMPX53DP_OpAmpGain 					= 50.0f;
const float kMPX53DP_kPA_cm						= 1.01f;

#define PIN_SERIAL_RX			0		// Serial port RX
#define PIN_SERIAL_TX			1		// Serial port TX

#define	PIN_REEDSWITCH_LOW		2		// Low manometer reed switch
#define	PIN_REEDSWITCH_MID		3		// Mid manometer reed switch
#define	PIN_REEDSWITCH_HIGH		4		// High manometer reed switch

#define PIN_OUT_LED				7		// LED debug output

#define PIN_OUT_PWM_PUMP		5		// Ambu pump Cam PWM output
#define PIN_OUT_PWM_BUZZER		9		// Buzzer PWM signal output

#define PIN_PRESSURE			A0		// Pressure readings from MPX53DP amplified through INA333 opamp



#endif // TLC_DEFS_H
