#include "control.h"
#include "datamodel.h"
#include "configuration.h"
#include "safeties.h"

bool Control_Init()
{
    return true;
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
	
	//*** Validate how we manage too much pressure
	if (gDataModel.fPI < 0)
	{
		gDataModel.fPI = 0;
	}
    
    gDataModel.nPWMPump = (uint8_t)(gDataModel.fPI * gConfiguration.fControlTransfer);
}

static bool ComputeRespirationSetPoint()
{
    bool timeTriggerEnable      = false;
    bool patientTriggerEnable   = false;
    switch (gDataModel.nTriggerMode)
    {
    case kTriggerMode_Timed:
        timeTriggerEnable   = true;
        break;
        
    case kTriggerMode_Patient:
        // If not in respiration cycle
        if (gDataModel.nRespirationCurveIndex >= gDataModel.nRespirationCurveCount)
        {
            // If patient triggers respiration
            if (gDataModel.fPressure_mmH2O[0] > gConfiguration.fPatientTrigger_mmH2O)
            {
                patientTriggerEnable = true;
            }
        }
        break;
    
    case kTriggerMode_PatientSemiAutomatic:
        timeTriggerEnable = true;
        // If not in respiration cycle
        if (gDataModel.nRespirationCurveIndex >= gDataModel.nRespirationCurveCount)
        {
            // If patient triggers respiration
            if (gDataModel.fPressure_mmH2O[0] > gConfiguration.fPatientTrigger_mmH2O)
            {
                patientTriggerEnable = true;
            }
        }
        break;
    
    default:
        // Invalid setting
        gSafeties.bConfigurationInvalid = true;
        gDataModel.nPWMPump             = 0;
        return false;
    };

    
    // Check for overflow of allowed maximum curve setpoint count
    if (gDataModel.nRespirationCurveIndex >= kMaxRespirationCurveCount)
    {
        // Raise a safety issue
        gSafeties.bCritical                 = true;
        gDataModel.nRespirationCurveIndex   = gDataModel.nRespirationCurveCount;
    }
        
    // Finished a cycle, then we wait for the next respiration
    if ((timeTriggerEnable || patientTriggerEnable) && gDataModel.nRespirationCurveIndex >= gDataModel.nRespirationCurveCount)
    {
        // Relaxation time, compared to respiration per minute (to milliseconds)
        if ((timeTriggerEnable && (gDataModel.nRespirationPerMinute != 0 && (millis() - gDataModel.nTickRespiration) >= (60000 / gDataModel.nRespirationPerMinute))) ||
            patientTriggerEnable)
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
			gDataModel.nTickSetPoint        = millis();
            ++gDataModel.nRespirationCurveIndex;
        }
    }
    
    return true;
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
    analogWrite(PIN_OUT_PWM_PUMP, gDataModel.nPWMPump);
}
