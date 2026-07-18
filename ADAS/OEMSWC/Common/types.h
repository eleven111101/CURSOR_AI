/******************************************************************************
 * @file    types.h
 * @brief   Common data type definitions for the ADAS Dependency Tracing POC
 *
 * This file contains all shared data types used across the OEM software
 * components including Camera, Radar, Fusion, FEB, HMI and Vehicle Status.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Basic Types
 ******************************************************************************/

typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef uint64_t    uint64;

typedef int8_t      sint8;
typedef int16_t     sint16;
typedef int32_t     sint32;
typedef int64_t     sint64;

typedef float       float32;
typedef double      float64;

/******************************************************************************
 * Generic Return Status
 ******************************************************************************/

typedef enum
{
    E_OK = 0,
    E_NOT_OK

} Std_ReturnType;

/******************************************************************************
 * Signal Quality
 ******************************************************************************/

typedef enum
{
    SIGNAL_QUALITY_INVALID = 0,
    SIGNAL_QUALITY_INIT,
    SIGNAL_QUALITY_LOW,
    SIGNAL_QUALITY_MEDIUM,
    SIGNAL_QUALITY_HIGH

} SignalQualityType;

/******************************************************************************
 * System State
 ******************************************************************************/

typedef enum
{
    SYSTEM_NOT_INITIALIZED = 0,
    SYSTEM_INITIALIZING,
    SYSTEM_OPERATIONAL,
    SYSTEM_DEGRADED,
    SYSTEM_FAILURE

} SystemStateType;

/******************************************************************************
 * Vehicle Motion State
 ******************************************************************************/

typedef enum
{
    VEHICLE_STOPPED = 0,
    VEHICLE_MOVING,
    VEHICLE_REVERSING

} VehicleMotionStateType;

/******************************************************************************
 * Gear Position
 ******************************************************************************/

typedef enum
{
    GEAR_PARK = 0,
    GEAR_REVERSE,
    GEAR_NEUTRAL,
    GEAR_DRIVE

} GearPositionType;

/******************************************************************************
 * Warning Status
 ******************************************************************************/

typedef enum
{
    WARNING_NONE = 0,
    WARNING_REQUESTED,
    WARNING_ACTIVE,
    WARNING_SUPPRESSED

} WarningStatusType;

/******************************************************************************
 * Brake Request
 ******************************************************************************/

typedef enum
{
    BRAKE_REQUEST_INACTIVE = 0,
    BRAKE_REQUEST_PARTIAL,
    BRAKE_REQUEST_FULL

} BrakeRequestType;

/******************************************************************************
 * Region Variant
 ******************************************************************************/

typedef enum
{
    REGION_EU = 0,
    REGION_US,
    REGION_APAC,
    REGION_ROW

} RegionType;

/******************************************************************************
 * Camera Object
 ******************************************************************************/

typedef struct
{
    uint16              objectId;
    float32             posX;
    float32             posY;
    float32             velocity;
    float32             confidence;
    SignalQualityType   quality;

} CameraObjectType;

/******************************************************************************
 * Radar Object
 ******************************************************************************/

typedef struct
{
    uint16              objectId;
    float32             range;
    float32             relativeSpeed;
    float32             angle;
    SignalQualityType   quality;

} RadarObjectType;

/******************************************************************************
 * Fusion Object
 ******************************************************************************/

typedef struct
{
    uint16              objectId;
    float32             distance;
    float32             relativeSpeed;
    float32             timeToCollision;
    float32             collisionProbability;
    bool                valid;

} FusionObjectType;

/******************************************************************************
 * FEB Output
 ******************************************************************************/

typedef struct
{
    WarningStatusType   warningStatus;
    BrakeRequestType    brakeRequest;
    SystemStateType     systemState;

} FEBOutputType;

/******************************************************************************
 * Vehicle Status
 ******************************************************************************/

typedef struct
{
    float32                 vehicleSpeed;
    float32                 steeringAngle;
    float32                 yawRate;

    GearPositionType        gearPosition;
    VehicleMotionStateType  motionState;

    bool                    ignitionOn;
    bool                    brakePedalPressed;

} VehicleStatusType;

/******************************************************************************
 * Diagnostic Information
 ******************************************************************************/

typedef struct
{
    uint16      dtc;
    uint32      timestamp;
    bool        active;

} DiagnosticInfoType;

/******************************************************************************
 * Calibration Parameters
 ******************************************************************************/

typedef struct
{
    float32 warningTTCThreshold;
    float32 brakeTTCThreshold;
    float32 minimumVehicleSpeed;

} CalibrationType;

/******************************************************************************
 * Feature Configuration
 ******************************************************************************/

typedef struct
{
    bool febEnabled;
    bool hmiEnabled;
    bool cameraEnabled;
    bool radarEnabled;

    RegionType region;

} FeatureConfigType;

/******************************************************************************
 * Global ADAS Context
 ******************************************************************************/

typedef struct
{
    VehicleStatusType   vehicleStatus;
    FEBOutputType       febOutput;
    CalibrationType     calibration;
    FeatureConfigType   featureConfig;

} ADASContextType;

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */