#include "control.h"
#include "datamodel.h"
#include "configuration.h"

bool Control_Init()
{
	return true;
}

void Control_PID()
{
	gDataModel.fPressureError = gDataModel.fPressure_cmH2O - gDataModel.fRequestPressure_cmH2O;
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
	
	gDataModel.nPWMPump = (uint8_t)(gDataModel.fPI * gConfiguration.fControlTransfer);
}

void Control_Process()
{
	if (gDataModel.nState != kState_Process)
	{
		analogWrite(PIN_OUT_PWM_PUMP, 0);
		return;
	}

	// Process pressure feedback to match current pressure set point
	Control_PID();
	
	// Pump power to output
	analogWrite(PIN_OUT_PWM_PUMP, gDataModel.nPWMPump);
}
