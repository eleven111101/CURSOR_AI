/******************************************************************************
 * @file    rte.c
 * @brief   Runtime Environment (RTE) Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "rte.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static ADASContextType gRTEContext;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void RTE_Init(void)
{
    memset(&gRTEContext, 0, sizeof(gRTEContext));

    gRTEContext.febOutput.systemState = SYSTEM_INITIALIZING;
}

void RTE_MainFunction(void)
{
    /* Placeholder for periodic RTE activities */

    if (gRTEContext.febOutput.systemState == SYSTEM_INITIALIZING)
    {
        gRTEContext.febOutput.systemState = SYSTEM_OPERATIONAL;
    }
}

/******************************************************************************
 * Vehicle Status Services
 ******************************************************************************/

Std_ReturnType RTE_WriteVehicleStatus(const VehicleStatusType *status)
{
    if (status == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(&gRTEContext.vehicleStatus,
           status,
           sizeof(VehicleStatusType));

    return E_OK;
}

Std_ReturnType RTE_ReadVehicleStatus(VehicleStatusType *status)
{
    if (status == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(status,
           &gRTEContext.vehicleStatus,
           sizeof(VehicleStatusType));

    return E_OK;
}

/******************************************************************************
 * FEB Output Services
 ******************************************************************************/

Std_ReturnType RTE_WriteFEBOutput(const FEBOutputType *output)
{
    if (output == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(&gRTEContext.febOutput,
           output,
           sizeof(FEBOutputType));

    return E_OK;
}

Std_ReturnType RTE_ReadFEBOutput(FEBOutputType *output)
{
    if (output == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(output,
           &gRTEContext.febOutput,
           sizeof(FEBOutputType));

    return E_OK;
}

/******************************************************************************
 * Calibration Services
 ******************************************************************************/

Std_ReturnType RTE_ReadCalibration(CalibrationType *calibration)
{
    if (calibration == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(calibration,
           &gRTEContext.calibration,
           sizeof(CalibrationType));

    return E_OK;
}

/******************************************************************************
 * Feature Configuration Services
 ******************************************************************************/

Std_ReturnType RTE_ReadFeatureConfig(FeatureConfigType *featureConfig)
{
    if (featureConfig == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(featureConfig,
           &gRTEContext.featureConfig,
           sizeof(FeatureConfigType));

    return E_OK;
}