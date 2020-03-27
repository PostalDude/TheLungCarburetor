#include "datamodel.h"

tDataModel gDataModel;

bool DataModel_Init()
{
    memset(&gDataModel, 0, sizeof(tDataModel));

	gDataModel.pInhaleCurve.nCount = 8;
	for (int a = 0; a < 8; ++a)
	{
		gDataModel.pInhaleCurve.nSetPoint_TickMs[a] = 50;
		gDataModel.pInhaleCurve.fSetPoint_mmH2O[a]	= a * 50;
	}

	gDataModel.pExhaleCurve.nCount = 8;
	for (int a = 0; a < 8; ++a)
	{
		gDataModel.pExhaleCurve.nSetPoint_TickMs[a] = 50;
		gDataModel.pExhaleCurve.fSetPoint_mmH2O[a]	= a * 5;
	}

	gDataModel.nRespirationPerMinute 	= 12;
	gDataModel.nControlMode 			= kControlMode_PID;
	gDataModel.nTriggerMode 			= kTriggerMode_Timed;

    gDataModel.nState = kState_Init;
    return true;
}

void DataModel_Process()
{
    if (gDataModel.nState != kState_Process)
    {
        return;
    }
}
