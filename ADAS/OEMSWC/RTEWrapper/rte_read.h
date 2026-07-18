/******************************************************************************
 * @file    rte_read.h
 * @brief   Runtime Environment Read APIs
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef RTE_READ_H
#define RTE_READ_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/******************************************************************************
 * Vehicle Status Read APIs
 ******************************************************************************/

/**
 * @brief Read the complete vehicle status.
 *
 * @param vehicleStatus Pointer to destination structure.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadVehicleStatusData(VehicleStatusType *vehicleStatus);

/**
 * @brief Read current vehicle speed.
 *
 * @param speed Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadVehicleSpeed(float32 *speed);

/**
 * @brief Read steering angle.
 *
 * @param steeringAngle Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadSteeringAngle(float32 *steeringAngle);

/**
 * @brief Read yaw rate.
 *
 * @param yawRate Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadYawRate(float32 *yawRate);

/**
 * @brief Read current gear position.
 *
 * @param gearPosition Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadGearPosition(GearPositionType *gearPosition);

/**
 * @brief Read vehicle motion state.
 *
 * @param motionState Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadMotionState(VehicleMotionStateType *motionState);

/**
 * @brief Read ignition status.
 *
 * @param ignitionOn Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadIgnitionStatus(bool *ignitionOn);

/**
 * @brief Read brake pedal status.
 *
 * @param brakePressed Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RTE_ReadBrakePedalStatus(bool *brakePressed);

/******************************************************************************
 * Calibration Read APIs
 ******************************************************************************/

Std_ReturnType RTE_ReadCalibrationData(CalibrationType *calibration);

/******************************************************************************
 * Feature Configuration Read APIs
 ******************************************************************************/

Std_ReturnType RTE_ReadFeatureConfiguration(FeatureConfigType *featureConfig);

/******************************************************************************
 * FEB Output Read APIs
 ******************************************************************************/

Std_ReturnType RTE_ReadFEBOutputData(FEBOutputType *febOutput);

#ifdef __cplusplus
}
#endif

#endif /* RTE_READ_H */