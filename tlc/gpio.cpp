#include "gpio.h"
#include "datamodel.h"
#include "control.h"

// Set ports direction
bool GPIO_Init()
{
    // Configure pins from defs.h
    pinMode(PIN_PRESSURE0, INPUT);
    pinMode(PIN_PRESSURE1, INPUT);
    pinMode(PIN_BATTERY,   INPUT);

    exhaleValveServo.attach(PIN_OUT_SERVO_EXHALE);

    // Set pin mode for buzzer
    pinMode(PIN_OUT_BUZZER, OUTPUT);
    digitalWrite(PIN_OUT_BUZZER, HIGH);

    pinMode(PIN_OUT_PUMP1_PWM, OUTPUT);
    digitalWrite(PIN_OUT_PUMP1_PWM, LOW);

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
