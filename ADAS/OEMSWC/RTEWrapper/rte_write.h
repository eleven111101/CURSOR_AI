/******************************************************************************
 * @file    rte_write.h
 * @brief   Runtime Environment Write APIs
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef RTE_WRITE_H
#define RTE_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/******************************************************************************
 * Vehicle Status Write APIs
 ******************************************************************************/

/**
 * @brief Write complete vehicle status.
 *
 * @param vehicleStatus Pointer to vehicle status.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_WriteVehicleStatusData(
    const VehicleStatusType *vehicleStatus);

/******************************************************************************
 * FEB Output Write APIs
 ******************************************************************************/

/**
 * @brief Write FEB output.
 *
 * @param febOutput Pointer to FEB output.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_WriteFEBOutputData(
    const FEBOutputType *febOutput);

/******************************************************************************
 * Calibration Write APIs
 ******************************************************************************/

/**
 * @brief Update calibration parameters.
 *
 * @param calibration Pointer to calibration structure.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_WriteCalibrationData(
    const CalibrationType *calibration);

/******************************************************************************
 * Feature Configuration Write APIs
 ******************************************************************************/

/**
 * @brief Update feature configuration.
 *
 * @param featureConfig Pointer to feature configuration.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_WriteFeatureConfiguration(
    const FeatureConfigType *featureConfig);

#ifdef __cplusplus
}
#endif

#endif /* RTE_WRITE_H */