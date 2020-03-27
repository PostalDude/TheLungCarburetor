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
    digitalWrite(PIN_OUT_BUZZER, LOW);

    pinMode(PIN_OUT_EMERGENCY_RELAY, OUTPUT);
    digitalWrite(PIN_OUT_BUZZER, HIGH);

    // Set pin mode for pressure pump
    pinMode(PIN_OUT_PUMP1_DIRA, OUTPUT);
    digitalWrite(PIN_OUT_PUMP1_DIRA, LOW);

    pinMode(PIN_OUT_PUMP1_DIRB, OUTPUT);
    digitalWrite(PIN_OUT_PUMP1_DIRB, LOW);

    pinMode(PIN_OUT_PUMP2_DIRA, OUTPUT);
    digitalWrite(PIN_OUT_PUMP2_DIRA, LOW);

    pinMode(PIN_OUT_PUMP2_DIRB, OUTPUT);
    digitalWrite(PIN_OUT_PUMP2_DIRB, LOW);

    pinMode(PIN_OUT_PUMP1_PWM, OUTPUT);
    analogWrite(PIN_OUT_PUMP1_PWM, 0);

    pinMode(PIN_OUT_PUMP2_PWM, OUTPUT);
    analogWrite(PIN_OUT_PUMP2_PWM, 0);

    pinMode(PIN_OUT_PUMPS_ENABLE, OUTPUT);
    digitalWrite(PIN_OUT_PUMPS_ENABLE, HIGH);

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
