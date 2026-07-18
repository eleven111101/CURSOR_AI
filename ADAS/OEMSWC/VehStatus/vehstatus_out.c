/******************************************************************************
 * @file    vehstatus_out.c
 * @brief   Vehicle Status Output Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "vehstatus_out.h"
#include "vehstatus_in.h"

#include <string.h>

/******************************************************************************
 * Global Functions
 ******************************************************************************/

float32 VehStatus_GetVehicleSpeed(void)
{
    return VehStatus_GetDatabase()->vehicleStatus.vehicleSpeed;
}

float32 VehStatus_GetSteeringAngle(void)
{
    return VehStatus_GetDatabase()->vehicleStatus.steeringAngle;
}

GearPositionType VehStatus_GetGearPosition(void)
{
    return VehStatus_GetDatabase()->vehicleStatus.gearPosition;
}

VehicleMotionStateType VehStatus_GetMotionState(void)
{
    return VehStatus_GetDatabase()->vehicleStatus.motionState;
}

bool VehStatus_IsBrakePressed(void)
{
    return VehStatus_GetDatabase()->vehicleStatus.brakePedalPressed;
}

void VehStatus_GetVehicleStatus(VehicleStatusType *status)
{
    if(status != NULL)
    {
        memcpy(status,
               &VehStatus_GetDatabase()->vehicleStatus,
               sizeof(VehicleStatusType));
    }
}

bool VehStatus_IsDataValid(void)
{
    return VehStatus_GetDatabase()->dataValid;
}