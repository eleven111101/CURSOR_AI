/******************************************************************************
 * @file    rte.h
 * @brief   Runtime Environment (RTE) Interface
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef RTE_H
#define RTE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize the Runtime Environment.
 */
void RTE_Init(void);

/**
 * @brief Execute periodic RTE processing.
 */
void RTE_MainFunction(void);

/******************************************************************************
 * Vehicle Status Services
 ******************************************************************************/

/**
 * @brief Update Vehicle Status in the RTE.
 *
 * @param status Pointer to vehicle status.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_WriteVehicleStatus(const VehicleStatusType *status);

/**
 * @brief Read Vehicle Status from the RTE.
 *
 * @param status Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadVehicleStatus(VehicleStatusType *status);

/******************************************************************************
 * FEB Output Services
 ******************************************************************************/

/**
 * @brief Update FEB output in the RTE.
 *
 * @param output Pointer to FEB output.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_WriteFEBOutput(const FEBOutputType *output);

/**
 * @brief Read FEB output from the RTE.
 *
 * @param output Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadFEBOutput(FEBOutputType *output);

/******************************************************************************
 * Calibration Services
 ******************************************************************************/

/**
 * @brief Read calibration parameters.
 *
 * @param calibration Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadCalibration(CalibrationType *calibration);

/******************************************************************************
 * Feature Configuration Services
 ******************************************************************************/

/**
 * @brief Read feature configuration.
 *
 * @param featureConfig Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadFeatureConfig(FeatureConfigType *featureConfig);

#ifdef __cplusplus
}
#endif

#endif /* RTE_H */