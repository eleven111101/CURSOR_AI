/******************************************************************************
 * @file    vehstatus_out.h
 * @brief   Vehicle Status Output Interface
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef VEHSTATUS_OUT_H
#define VEHSTATUS_OUT_H

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
 * @brief Get current vehicle speed.
 *
 * @return Vehicle speed in km/h.
 */
float32 VehStatus_GetVehicleSpeed(void);

/**
 * @brief Get steering wheel angle.
 *
 * @return Steering angle in degrees.
 */
float32 VehStatus_GetSteeringAngle(void);

/**
 * @brief Get current gear position.
 *
 * @return Gear position.
 */
GearPositionType VehStatus_GetGearPosition(void);

/**
 * @brief Get vehicle motion state.
 *
 * @return Vehicle motion state.
 */
VehicleMotionStateType VehStatus_GetMotionState(void);

/**
 * @brief Check brake pedal status.
 *
 * @return true if brake pedal is pressed.
 */
bool VehStatus_IsBrakePressed(void);

/**
 * @brief Get complete vehicle status.
 *
 * @param status Pointer to destination structure.
 */
void VehStatus_GetVehicleStatus(VehicleStatusType *status);

/**
 * @brief Check if vehicle status database is valid.
 *
 * @return true if valid.
 */
bool VehStatus_IsDataValid(void);

#ifdef __cplusplus
}
#endif

#endif /* VEHSTATUS_OUT_H */