#include "gpio.h"
#include "datamodel.h"

// Set ports direction
bool GPIO_Init()
{
	// Configure pins from defs.h		
	pinMode(PIN_PRESSURE, INPUT);
	
	// Reed switches are normally open and connected to ground when closed, so they will read High when opened
	pinMode(PIN_REEDSWITCH_LOW,		INPUT_PULLUP);
	pinMode(PIN_REEDSWITCH_MID,		INPUT_PULLUP);
	pinMode(PIN_REEDSWITCH_HIGH,	INPUT_PULLUP);

	// Set led output pin
	pinMode(PIN_OUT_LED, OUTPUT);
	digitalWrite(PIN_OUT_LED, HIGH);

	// Set pin mode for buzzer
	pinMode(PIN_OUT_PWM_BUZZER, OUTPUT); 
	
	// Set pin mode for pressure pump
	pinMode(PIN_OUT_PWM_PUMP, OUTPUT);
	analogWrite(PIN_OUT_PWM_PUMP, 0);

	return true;
}

// System process
void GPIO_Process()
{
	if (gDataModel.nState != kState_Process)
	{
		return;
	}

	// Read reed switch and inverse signal so that 0 == Opened, 1 == Closed
	uint8_t limit0 = digitalRead(PIN_REEDSWITCH_LOW)  ^ 1;
	uint8_t limit1 = digitalRead(PIN_REEDSWITCH_MID)  ^ 1;
	uint8_t limit2 = digitalRead(PIN_REEDSWITCH_HIGH) ^ 1;
	
	gDataModel.nPressureLimits = limit0 | (limit1 << 1) | (limit2 << 2);
}
