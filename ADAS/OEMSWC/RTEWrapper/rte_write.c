/******************************************************************************
 * @file    rte_write.c
 * @brief   Runtime Environment Write API Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "rte_write.h"
#include "rte.h"

#include <string.h>

/******************************************************************************
 * Calibration Storage
 ******************************************************************************/

static CalibrationType gCalibrationData;

/******************************************************************************
 * Feature Configuration Storage
 ******************************************************************************/

static FeatureConfigType gFeatureConfig;

/******************************************************************************
 * Vehicle Status Write APIs
 ******************************************************************************/

Std_ReturnType RTE_WriteVehicleStatusData(
    const VehicleStatusType *vehicleStatus)
{
    return RTE_WriteVehicleStatus(vehicleStatus);
}

/******************************************************************************
 * FEB Output Write APIs
 ******************************************************************************/

Std_ReturnType RTE_WriteFEBOutputData(
    const FEBOutputType *febOutput)
{
    return RTE_WriteFEBOutput(febOutput);
}

/******************************************************************************
 * Calibration Write APIs
 ******************************************************************************/

Std_ReturnType RTE_WriteCalibrationData(
    const CalibrationType *calibration)
{
    if (calibration == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(&gCalibrationData,
           calibration,
           sizeof(CalibrationType));

    return E_OK;
}

/******************************************************************************
 * Feature Configuration Write APIs
 ******************************************************************************/

Std_ReturnType RTE_WriteFeatureConfiguration(
    const FeatureConfigType *featureConfig)
{
    if (featureConfig == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(&gFeatureConfig,
           featureConfig,
           sizeof(FeatureConfigType));

    return E_OK;
}