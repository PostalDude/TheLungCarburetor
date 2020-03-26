#include "gpio.h"
#include "datamodel.h"

// Set ports direction
bool GPIO_Init()
{
    // Configure pins from defs.h       
    pinMode(PIN_PRESSURE0, INPUT);
    pinMode(PIN_PRESSURE1, INPUT);
	pinMode(PIN_BATTERY,   INPUT);
    
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
}
