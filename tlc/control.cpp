#include "control.h"
#include "datamodel.h"
#include "configuration.h"
#include "safeties.h"

bool Control_Init()
{
    return true;
}

// Control using a PID and use feedback from patient own respiration
void Control_AssistedPID()
{
}

// Control without using the pressure feedback
void Control_OpenLoop()
{
}

// Control using a PID with pressure feedback
void Control_PID()
{
    gDataModel.fPressureError = gDataModel.fPressure_mmH2O[0] - gDataModel.fRequestPressure_mmH2O;
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
    
    gDataModel.nPWMPump = (uint8_t)(gDataModel.fPI * gConfiguration.fControlTransfer);
}

static void ComputeRespirationSetPoint()
{
    // Check for overflow of allowed maximum curve setpoint count
    if (gDataModel.nRespirationCurveIndex >= kMaxRespirationCurveCount)
    {
        // Raise a safety issue
        gSafeties.bCritical                 = true;
        gDataModel.nRespirationCurveIndex   = gDataModel.nRespirationCurveCount;
    }
        
    // Finished a cycle, then we wait for the next respiration
    if (gDataModel.nRespirationCurveIndex >= gDataModel.nRespirationCurveCount)
    {
        // Relaxation time, compared to respiration per minute (to milliseconds)
        if ((millis() - gDataModel.nTickRespiration) >= (gDataModel.nRespirationPerMinute * 60000))
        {
            // Start a new respiration cycle
            gDataModel.nRespirationCurveIndex   = 0;
            gDataModel.nTickSetPoint            = millis();
            gDataModel.nTickRespiration         = millis();
            gDataModel.fRequestPressure_mmH2O   = gDataModel.fRespirationCurve_mmH2O[0];
        }
    }
    else
    {
        gDataModel.fRequestPressure_mmH2O   = gDataModel.fRespirationCurve_mmH2O[gDataModel.nRespirationCurveIndex];
        
        if ((millis() - gDataModel.nTickSetPoint) >= gDataModel.nRespirationCurve_Tick[gDataModel.nTickSetPoint])
        {
            ++gDataModel.nRespirationCurveIndex;
        }
    }
}

void Control_Process()
{
    if (gDataModel.nState != kState_Process)
    {
        analogWrite(PIN_OUT_PWM_PUMP, 0);
        return;
    }
    
    // Process pressure feedback to match current pressure set point
    switch (gDataModel.nControlMode)
    {
    case kControlMode_PID:
        ComputeRespirationSetPoint();
        Control_PID();
        break;  
        
    case kControlMode_AssistedPID:
        ComputeRespirationSetPoint();
        Control_AssistedPID();
        break;
        
    case kControlMode_OpenLoop:     
        ComputeRespirationSetPoint();
        Control_OpenLoop();
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
    analogWrite(PIN_OUT_PWM_PUMP, gDataModel.nPWMPump);
}
