#include "sensors.h"
#include "datamodel.h"
#include "configuration.h"

// Initialize sensor devices
bool Sensors_Init()
{
	return true;
}

// Process sensors sampling
void Sensors_Process()
{
	if (gDataModel.nState != kState_Process)
	{
		return;
	}

	gDataModel.nRawPressure = analogRead(PIN_PRESSURE) - gConfiguration.nPressureSensorOffset;	
	
	// Transfer function for raw pressure.
	float voltage  = (float)gDataModel.nRawPressure / 1023.0f * 3.3f;	// 3.3 volts INA333
	voltage		  *= 1.0f / kMPX53DP_OpAmpGain;
	float pressure = voltage / kMPX53DP_SensitivityVoltPerKPA;   		// Differential pressure in kPa
	
	gDataModel.fPressure_cmH2O = pressure * kMPX53DP_kPA_cm;   		// kPa to cm
}
