#include "control.h"
#include "datamodel.h"
#include "configuration.h"
#include "safeties.h"
#include "TimerOne.h"
#include "lcd_keypad.h"

ServoTimer2 exhaleValveServo;

bool Control_Init()
{
    gDataModel.nTickRespiration = millis(); // Respiration cycle start tick. Used to compute respiration per minutes
    exhaleValveServo.write(gConfiguration.nServoExhaleOpenAngle);

    Timer1.initialize(4000);         // initialize timer1, and set a 4000us period
    Timer1.pwm(PIN_OUT_PUMP1_PWM, 0);                // setup pwm on pin 9, 50% duty cycle  ( 0 to 1000)

	gDataModel.bStartFlag = false;
	
    return true;
}

// Control using a PID with pressure feedback
void Control_PID()
{
    gDataModel.fPressureError = gDataModel.fRequestPressure_mmH2O - gDataModel.fPressure_mmH2O[0];
    gDataModel.fP = gDataModel.fPressureError * gConfiguration.fGainP;

    gDataModel.fI += gDataModel.fPressureError;
    if (gDataModel.fI > gConfiguration.fILimit)
    {
        gDataModel.fI = gConfiguration.fILimit;
    }
    else if (gDataModel.fI < -gConfiguration.fILimit)
    {
        gDataModel.fI = -gConfiguration.fILimit;
    }

    gDataModel.fPI = gDataModel.fP + gDataModel.fI;
    if (gDataModel.fPI > gConfiguration.fPILimit)
    {
        gDataModel.fPI = gConfiguration.fPILimit;
    }
    else if (gDataModel.fPI < -gConfiguration.fPILimit)
    {
        gDataModel.fPI = -gConfiguration.fPILimit;
    }

    // Note: Derivative not used, not necessary for now.

    //*** Validate how we manage too much pressure
    if (gDataModel.fPI < 0)
    {
        gDataModel.fPI = 0;
    }

    gDataModel.nPWMPump = (uint16_t)(gDataModel.fPI * gConfiguration.fControlTransfer);
}

// Return true when condition for respiration has been triggered
static bool CheckTrigger()
{
    bool trigger = false;
    switch (gDataModel.nTriggerMode)
    {
    case kTriggerMode_Timed:
        trigger = ((gDataModel.nRespirationPerMinute > 0) && (millis() - gDataModel.nTickRespiration) >= (60000 / gDataModel.nRespirationPerMinute));
        break;

    case kTriggerMode_Patient:
        // If patient triggers respiration
        if (gDataModel.fPressure_mmH2O[0] < gConfiguration.fPatientTrigger_mmH2O)
        {
            trigger = true;
        }
        break;

    case kTriggerMode_PatientSemiAutomatic:
        // If patient triggers respiration or if time for next respiration cycle
        if (gDataModel.fPressure_mmH2O[0] < gConfiguration.fPatientTrigger_mmH2O)
        {
            trigger = true;
        }
        trigger |= ((gDataModel.nRespirationPerMinute > 0) && (millis() - gDataModel.nTickRespiration) >= (60000 / gDataModel.nRespirationPerMinute));
        break;

    default:
        // Invalid setting
        gSafeties.bConfigurationInvalid = true;
        gDataModel.nPWMPump             = 0;
        return false;
    };

    return trigger;
}

static bool BeginRespirationCycle()
{
    gDataModel.nTickRespiration = millis(); // Respiration cycle start tick. Used to compute respiration per minutes

    return true;
}

static bool EndRespirationCycle()
{
    return true;
}

static bool StartInhaleCycle()
{
    if (gDataModel.pInhaleCurve.nCount <= 0)
    {
        return false;
    }

    // Make sure exhale valve is closed
    exhaleValveServo.write(gConfiguration.nServoExhaleCloseAngle);

    // Start a new inhale cycle
    gDataModel.nCurveIndex              = 0;
    gDataModel.nTickSetPoint            = millis();
    gDataModel.fRequestPressure_mmH2O   = gDataModel.pInhaleCurve.fSetPoint_mmH2O[0];

    return true;
}

// Inhale cycle set point, returns true when cycle is finished
static bool Inhale()
{
    // Check for overflow of allowed maximum curve setpoint count
    if (gDataModel.nCurveIndex >= kMaxCurveCount)
    {
        // Raise a safety issue
        gSafeties.bCritical     = true;
        gDataModel.nCurveIndex  = gDataModel.pInhaleCurve.nCount;
    }

    if (gDataModel.nCurveIndex >= gDataModel.pInhaleCurve.nCount)
    {
        return true;
    }

    gDataModel.fRequestPressure_mmH2O = gDataModel.pInhaleCurve.fSetPoint_mmH2O[gDataModel.nCurveIndex];
    if ((millis() - gDataModel.nTickSetPoint) >= gDataModel.pInhaleCurve.nSetPoint_TickMs[gDataModel.nCurveIndex])
    {
        gDataModel.nTickSetPoint = millis();
        ++gDataModel.nCurveIndex;
    }

    return false;
}

static bool StopInhaleCycle()
{
    return true;
}

static bool StartExhaleCycle()
{
    if (gDataModel.pExhaleCurve.nCount <= 0)
    {
        return false;
    }

    // Start a new inhale cycle
    gDataModel.nCurveIndex              = 0;
    gDataModel.nTickSetPoint            = millis();
    gDataModel.fRequestPressure_mmH2O   = gDataModel.pExhaleCurve.fSetPoint_mmH2O[0];

    exhaleValveServo.write(gConfiguration.nServoExhaleOpenAngle);

    return true;
}

static bool Exhale()
{
    // Check for overflow of allowed maximum curve setpoint count
    if (gDataModel.nCurveIndex >= kMaxCurveCount)
    {
        // Raise a safety issue
        gSafeties.bCritical     = true;
        gDataModel.nCurveIndex  = gDataModel.pExhaleCurve.nCount;
    }

    if (gDataModel.nCurveIndex >= gDataModel.pExhaleCurve.nCount)
    {
        return true;
    }

    gDataModel.fRequestPressure_mmH2O = gDataModel.pExhaleCurve.fSetPoint_mmH2O[gDataModel.nCurveIndex];
    if ((millis() - gDataModel.nTickSetPoint) >= gDataModel.pExhaleCurve.nSetPoint_TickMs[gDataModel.nCurveIndex])
    {
        gDataModel.nTickSetPoint = millis();
        ++gDataModel.nCurveIndex;
    }

    return false;
}

static bool StopExhaleCycle()
{
    exhaleValveServo.write(gConfiguration.nServoExhaleCloseAngle);

    return true;
}

static bool ComputeRespirationSetPoint()
{
    // Process proper part of the respiration cycle: Trigger, Inhale or Exhale
    switch (gDataModel.nCycleState)
    {
    case kCycleState_WaitTrigger:
        sprintf(gLcdDetail, "Trigger   ");
        if (CheckTrigger())
        {
            BeginRespirationCycle();
            if (StartInhaleCycle())
            {
                gDataModel.nCycleState = kCycleState_Inhale;
            }
            else
            {
                // No inhale curve set
                gSafeties.bCritical = true;
            }
        }
        break;

    case kCycleState_Inhale:
        {
            sprintf(gLcdDetail, "Inhale  ");
            bool inhaleFinished = Inhale();
            if (inhaleFinished)
            {
                StopInhaleCycle();
                if (StartExhaleCycle())
                {
                    gDataModel.nCycleState = kCycleState_Exhale;
                }
            }
            else
            {
                // No exhale curve set
                gSafeties.bCritical = true;
            }
        }
        break;

    case kCycleState_Exhale:
        {
            sprintf(gLcdDetail, "Exhale   ");
            bool exhaleFinished = Exhale();
            if (exhaleFinished)
            {
                StopExhaleCycle();
                EndRespirationCycle();
                gDataModel.nTickStabilization = millis();
                gDataModel.nCycleState = kCycleState_Stabilization;
            }
        }
        break;

    case kCycleState_Stabilization:
        sprintf(gLcdDetail, "Stabil   ");
        // Pressure Stabilization between cycles
        if ((millis() - gDataModel.nTickStabilization) >= kPeriodStabilization)
        {
            gDataModel.nCycleState = kCycleState_WaitTrigger;
        }
        break;

    default:
        sprintf(gLcdDetail, "N/A   ");
        // Invalid setting
        gSafeties.bConfigurationInvalid = true;
        gDataModel.nPWMPump             = 0;
        gDataModel.nCycleState          = kCycleState_WaitTrigger;
        return false;
    };

    return true;
}

void Control_Process()
{
    if (!gDataModel.bStartFlag || gDataModel.nState != kState_Process)
    {        
        gDataModel.nCycleState = kCycleState_WaitTrigger;
		exhaleValveServo.write(gConfiguration.nServoExhaleOpenAngle);
		gDataModel.nTickRespiration = millis(); // Respiration cycle start tick. Used to compute 
        Timer1.pwm(PIN_OUT_PUMP1_PWM, gDataModel.nPWMPump);		
        return;
    }

    // Process pressure feedback to match current pressure set point
    switch (gDataModel.nControlMode)
    {
    case kControlMode_PID:
        // It is assumed that the last pressure setpoint in the exhale curve is kept between respiration
        if (ComputeRespirationSetPoint())
        {
            Control_PID();
        }
        break;

    case kControlMode_FeedForward:
        // In feedforward, the master controls the gDataModel.nPWMPump value through the serial port
        break;

    default:
        // Unknown control mode, raise error.
        gSafeties.bConfigurationInvalid = true;
        gDataModel.nPWMPump             = 0;
        break;
    };

    // Pump power to output
    Timer1.pwm(PIN_OUT_PUMP1_PWM, gDataModel.nPWMPump);
}
