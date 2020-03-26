#ifndef TLC_CONTROL_H
#define TLC_CONTROL_H

#include <Servo.h>
extern Servo exhaleValveServo;

bool Control_Init();
void Control_Process();

#endif // TLC_CONTROL_H
