/******************************************************************************
 * @file    feb_logic.c
 * @brief   Forward Emergency Braking Decision Logic Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "feb_logic.h"
#include "calibration.h"

#include <string.h>

/******************************************************************************
 * Local Function Prototypes
 ******************************************************************************/

static const FusionObjectType* FEBLogic_FindCriticalObject(
    const FusionObjectListType *fusionObjects);

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void FEBLogic_Init(void)
{
}

void FEBLogic_MainFunction(void)
{
}

Std_ReturnType FEBLogic_Process(
    const FusionObjectListType *fusionObjects,
    FEBOutputType *output)
{
    const FusionObjectType *criticalObject;
    const FEBCalibrationType *calibration;

    if ((fusionObjects == NULL) ||
        (output == NULL))
    {
        return E_NOT_OK;
    }

    calibration = Calibration_GetFEBCalibration();

    if (calibration == NULL)
    {
        return E_NOT_OK;
    }

    memset(output, 0, sizeof(FEBOutputType));

    output->systemState = SYSTEM_OPERATIONAL;
    output->warningStatus = WARNING_NONE;
    output->brakeRequest = BRAKE_REQUEST_INACTIVE;

    criticalObject = FEBLogic_FindCriticalObject(fusionObjects);

    if (criticalObject == NULL)
    {
        return E_OK;
    }

    if (criticalObject->timeToCollision <= calibration->brakeTTC)
    {
        output->warningStatus = WARNING_ACTIVE;
        output->brakeRequest = BRAKE_REQUEST_FULL;
    }
    else if (criticalObject->timeToCollision <= calibration->warningTTC)
    {
        output->warningStatus = WARNING_REQUESTED;
        output->brakeRequest = BRAKE_REQUEST_PARTIAL;
    }

    return E_OK;
}

void FEBLogic_Reset(void)
{
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

static const FusionObjectType* FEBLogic_FindCriticalObject(
    const FusionObjectListType *fusionObjects)
{
    const FusionObjectType *criticalObject = NULL;
    uint8 index;

    if (fusionObjects == NULL)
    {
        return NULL;
    }

    for (index = 0U; index < fusionObjects->objectCount; index++)
    {
        if (!fusionObjects->objects[index].valid)
        {
            continue;
        }

        if ((criticalObject == NULL) ||
            (fusionObjects->objects[index].timeToCollision <
             criticalObject->timeToCollision))
        {
            criticalObject = &fusionObjects->objects[index];
        }
    }

    return criticalObject;
}