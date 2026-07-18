/******************************************************************************
 * @file    common.c
 * @brief   Common utility function implementation
 ******************************************************************************/

#include "common.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

/******************************************************************************
 * Mathematical Utilities
 ******************************************************************************/

float32 Common_CalculateDistance(float32 x1,
                                 float32 y1,
                                 float32 x2,
                                 float32 y2)
{
    float32 dx = x2 - x1;
    float32 dy = y2 - y1;

    return sqrtf((dx * dx) + (dy * dy));
}

float32 Common_CalculateRelativeSpeed(float32 objectSpeed,
                                      float32 vehicleSpeed)
{
    return vehicleSpeed - objectSpeed;
}

float32 Common_CalculateTimeToCollision(float32 distance,
                                        float32 relativeSpeed)
{
    if(relativeSpeed <= 0.0F)
    {
        return 9999.0F;
    }

    return distance / relativeSpeed;
}

float32 Common_ClampFloat(float32 value,
                          float32 minimum,
                          float32 maximum)
{
    if(value < minimum)
    {
        return minimum;
    }

    if(value > maximum)
    {
        return maximum;
    }

    return value;
}

/******************************************************************************
 * Validation
 ******************************************************************************/

bool Common_IsVehicleSpeedValid(float32 speed)
{
    return (speed >= VEHICLE_SPEED_MIN_KPH) &&
           (speed <= VEHICLE_SPEED_MAX_KPH);
}

bool Common_IsCameraObjectValid(const CameraObjectType *object)
{
    if(object == NULL_PTR)
    {
        return false;
    }

    return (object->confidence >= DEFAULT_MIN_CONFIDENCE);
}

bool Common_IsRadarObjectValid(const RadarObjectType *object)
{
    return (object != NULL_PTR);
}

bool Common_IsFusionObjectValid(const FusionObjectType *object)
{
    return (object != NULL_PTR) &&
           (object->valid == true);
}

/******************************************************************************
 * Reset Functions
 ******************************************************************************/

void Common_ResetCameraObject(CameraObjectType *object)
{
    if(object != NULL_PTR)
    {
        memset(object, 0, sizeof(CameraObjectType));
    }
}

void Common_ResetRadarObject(RadarObjectType *object)
{
    if(object != NULL_PTR)
    {
        memset(object, 0, sizeof(RadarObjectType));
    }
}

void Common_ResetFusionObject(FusionObjectType *object)
{
    if(object != NULL_PTR)
    {
        memset(object, 0, sizeof(FusionObjectType));
    }
}

void Common_ResetVehicleStatus(VehicleStatusType *status)
{
    if(status != NULL_PTR)
    {
        memset(status, 0, sizeof(VehicleStatusType));
    }
}

void Common_ResetFEBOutput(FEBOutputType *output)
{
    if(output != NULL_PTR)
    {
        memset(output, 0, sizeof(FEBOutputType));
    }
}

/******************************************************************************
 * Copy Functions
 ******************************************************************************/

void Common_CopyVehicleStatus(VehicleStatusType *destination,
                              const VehicleStatusType *source)
{
    if((destination != NULL_PTR) &&
       (source != NULL_PTR))
    {
        memcpy(destination,
               source,
               sizeof(VehicleStatusType));
    }
}

void Common_CopyFEBOutput(FEBOutputType *destination,
                          const FEBOutputType *source)
{
    if((destination != NULL_PTR) &&
       (source != NULL_PTR))
    {
        memcpy(destination,
               source,
               sizeof(FEBOutputType));
    }
}

/******************************************************************************
 * Alive Counter
 ******************************************************************************/

uint8 Common_UpdateAliveCounter(uint8 counter)
{
    return ALIVE_COUNTER_INCREMENT(counter);
}

/******************************************************************************
 * Diagnostics
 ******************************************************************************/

bool Common_IsSignalTimeout(uint32 currentTime,
                            uint32 previousTime,
                            uint32 timeout)
{
    return ((currentTime - previousTime) >= timeout);
}

void Common_ClearDiagnosticInfo(DiagnosticInfoType *diagnostic)
{
    if(diagnostic != NULL_PTR)
    {
        memset(diagnostic,
               0,
               sizeof(DiagnosticInfoType));
    }
}

/******************************************************************************
 * Logging
 ******************************************************************************/

void Common_LogInfo(const char *message)
{
    printf("[INFO] %s\n", message);
}

void Common_LogWarning(const char *message)
{
    printf("[WARNING] %s\n", message);
}

void Common_LogError(const char *message)
{
    printf("[ERROR] %s\n", message);
}