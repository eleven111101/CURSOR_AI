/******************************************************************************
 * @file    vehstatus_in.c
 * @brief   Vehicle Status Input Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "vehstatus_in.h"

#include "common.h"
#include "constants.h"

#include <string.h>

/******************************************************************************
 * Local Data
 ******************************************************************************/

static VehicleStatusDatabaseType gVehicleStatusDb;

/******************************************************************************
 * Local Function Prototypes
 ******************************************************************************/

static void VehStatus_UpdateMotionState(void);
static void VehStatus_UpdateValidity(void);

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void VehStatus_Init(void)
{
    memset(&gVehicleStatusDb, 0, sizeof(gVehicleStatusDb));

    gVehicleStatusDb.dataValid = false;
    gVehicleStatusDb.updateCounter = 0U;
    gVehicleStatusDb.lastUpdateTime = 0U;
}

void VehStatus_UpdateSignals(const DecodedVehicleSignalsType *signals)
{
    if(signals == NULL_PTR)
    {
        return;
    }

    gVehicleStatusDb.vehicleStatus.vehicleSpeed =
        signals->vehicleSpeedKph;

    gVehicleStatusDb.vehicleStatus.steeringAngle =
        signals->steeringAngleDeg;

    gVehicleStatusDb.vehicleStatus.brakePedalPressed =
        signals->brakePedalPressed;

    gVehicleStatusDb.vehicleStatus.gearPosition =
        signals->gearPosition;

    gVehicleStatusDb.vehicleStatus.motionState =
        signals->motionState;

    gVehicleStatusDb.updateCounter++;

    VehStatus_UpdateMotionState();

    VehStatus_UpdateValidity();
}

void VehStatus_MainFunction(void)
{
    VehStatus_UpdateValidity();
}

const VehicleStatusDatabaseType* VehStatus_GetDatabase(void)
{
    return &gVehicleStatusDb;
}

bool VehStatus_IsValid(void)
{
    return gVehicleStatusDb.dataValid;
}

void VehStatus_Reset(void)
{
    memset(&gVehicleStatusDb, 0, sizeof(gVehicleStatusDb));

    gVehicleStatusDb.dataValid = false;
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

static void VehStatus_UpdateMotionState(void)
{
    if(gVehicleStatusDb.vehicleStatus.vehicleSpeed > 0.1F)
    {
        gVehicleStatusDb.vehicleStatus.motionState =
            VEHICLE_MOVING;
    }
    else
    {
        gVehicleStatusDb.vehicleStatus.motionState =
            VEHICLE_STOPPED;
    }
}

static void VehStatus_UpdateValidity(void)
{
    gVehicleStatusDb.dataValid =
        Common_IsVehicleSpeedValid(
            gVehicleStatusDb.vehicleStatus.vehicleSpeed);
}