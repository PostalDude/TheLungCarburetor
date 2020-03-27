#ifndef TLC_CONTROL_H
#define TLC_CONTROL_H

#include "common.h"
#include <ServoTimer2.h>
extern ServoTimer2 exhaleValveServo;

bool Control_Init();
void Control_Process();

#endif // TLC_CONTROL_H
