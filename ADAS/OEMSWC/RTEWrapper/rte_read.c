/******************************************************************************
 * @file    rte_read.c
 * @brief   Runtime Environment Read API Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "rte_read.h"
#include "rte.h"

/******************************************************************************
 * Vehicle Status Read APIs
 ******************************************************************************/

Std_ReturnType RTE_ReadVehicleStatusData(VehicleStatusType *vehicleStatus)
{
    return RTE_ReadVehicleStatus(vehicleStatus);
}

Std_ReturnType RTE_ReadVehicleSpeed(float32 *speed)
{
    VehicleStatusType vehicleStatus;

    if ((speed == NULL) ||
        (RTE_ReadVehicleStatus(&vehicleStatus) != E_OK))
    {
        return E_NOT_OK;
    }

    *speed = vehicleStatus.vehicleSpeed;

    return E_OK;
}

Std_ReturnType RTE_ReadSteeringAngle(float32 *steeringAngle)
{
    VehicleStatusType vehicleStatus;

    if ((steeringAngle == NULL) ||
        (RTE_ReadVehicleStatus(&vehicleStatus) != E_OK))
    {
        return E_NOT_OK;
    }

    *steeringAngle = vehicleStatus.steeringAngle;

    return E_OK;
}

Std_ReturnType RTE_ReadYawRate(float32 *yawRate)
{
    VehicleStatusType vehicleStatus;

    if ((yawRate == NULL) ||
        (RTE_ReadVehicleStatus(&vehicleStatus) != E_OK))
    {
        return E_NOT_OK;
    }

    *yawRate = vehicleStatus.yawRate;

    return E_OK;
}

Std_ReturnType RTE_ReadGearPosition(GearPositionType *gearPosition)
{
    VehicleStatusType vehicleStatus;

    if ((gearPosition == NULL) ||
        (RTE_ReadVehicleStatus(&vehicleStatus) != E_OK))
    {
        return E_NOT_OK;
    }

    *gearPosition = vehicleStatus.gearPosition;

    return E_OK;
}

Std_ReturnType RTE_ReadMotionState(VehicleMotionStateType *motionState)
{
    VehicleStatusType vehicleStatus;

    if ((motionState == NULL) ||
        (RTE_ReadVehicleStatus(&vehicleStatus) != E_OK))
    {
        return E_NOT_OK;
    }

    *motionState = vehicleStatus.motionState;

    return E_OK;
}

Std_ReturnType RTE_ReadIgnitionStatus(bool *ignitionOn)
{
    VehicleStatusType vehicleStatus;

    if ((ignitionOn == NULL) ||
        (RTE_ReadVehicleStatus(&vehicleStatus) != E_OK))
    {
        return E_NOT_OK;
    }

    *ignitionOn = vehicleStatus.ignitionOn;

    return E_OK;
}

Std_ReturnType RTE_ReadBrakePedalStatus(bool *brakePressed)
{
    VehicleStatusType vehicleStatus;

    if ((brakePressed == NULL) ||
        (RTE_ReadVehicleStatus(&vehicleStatus) != E_OK))
    {
        return E_NOT_OK;
    }

    *brakePressed = vehicleStatus.brakePedalPressed;

    return E_OK;
}

/******************************************************************************
 * Calibration Read APIs
 ******************************************************************************/

Std_ReturnType RTE_ReadCalibrationData(CalibrationType *calibration)
{
    return RTE_ReadCalibration(calibration);
}

/******************************************************************************
 * Feature Configuration Read APIs
 ******************************************************************************/

Std_ReturnType RTE_ReadFeatureConfiguration(FeatureConfigType *featureConfig)
{
    return RTE_ReadFeatureConfig(featureConfig);
}

/******************************************************************************
 * FEB Output Read APIs
 ******************************************************************************/

Std_ReturnType RTE_ReadFEBOutputData(FEBOutputType *febOutput)
{
    return RTE_ReadFEBOutput(febOutput);
}