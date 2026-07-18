/******************************************************************************
 * @file    supplier_feb.c
 * @brief   Supplier Forward Emergency Braking Implementation
 *
 * Simulates a third-party supplier implementation of the Forward Emergency
 * Braking algorithm.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "supplier_feb.h"

#include "calibration.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static FEBOutputType gSupplierOutput;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void SupplierFEB_Init(void)
{
    memset(&gSupplierOutput, 0, sizeof(gSupplierOutput));
}

Std_ReturnType SupplierFEB_Process(
    const FusionObjectType *fusionObject,
    FEBOutputType *output)
{
    const FEBCalibrationType *calibration;

    if ((fusionObject == NULL) || (output == NULL))
    {
        return E_NOT_OK;
    }

    calibration = Calibration_GetFEBCalibration();

    memset(&gSupplierOutput, 0, sizeof(gSupplierOutput));

    gSupplierOutput.systemState = SYSTEM_OPERATIONAL;

    if (fusionObject->valid == true)
    {
        if (fusionObject->timeToCollision <= calibration->brakeTTC)
        {
            gSupplierOutput.warningStatus = true;
            gSupplierOutput.brakeRequest = true;
        }
        else if (fusionObject->timeToCollision <= calibration->warningTTC)
        {
            gSupplierOutput.warningStatus = true;
            gSupplierOutput.brakeRequest = false;
        }
    }

    memcpy(output,
           &gSupplierOutput,
           sizeof(FEBOutputType));

    return E_OK;
}

void SupplierFEB_Reset(void)
{
    memset(&gSupplierOutput, 0, sizeof(gSupplierOutput));
}