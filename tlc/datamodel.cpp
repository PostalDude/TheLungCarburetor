#include "datamodel.h"

tDataModel gDataModel;

bool DataModel_Init()
{
    memset(&gDataModel, 0, sizeof(tDataModel));

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

