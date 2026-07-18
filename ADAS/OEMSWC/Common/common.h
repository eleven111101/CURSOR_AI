/******************************************************************************
 * @file    common.h
 * @brief   Common utility function declarations
 *
 * This file contains generic helper functions shared across all software
 * components in the ADAS Dependency Tracing POC.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "macros.h"
#include "constants.h"

/******************************************************************************
 * Mathematical Utilities
 ******************************************************************************/

float32 Common_CalculateDistance(float32 x1,
                                 float32 y1,
                                 float32 x2,
                                 float32 y2);

float32 Common_CalculateRelativeSpeed(float32 objectSpeed,
                                      float32 vehicleSpeed);

float32 Common_CalculateTimeToCollision(float32 distance,
                                        float32 relativeSpeed);

float32 Common_ClampFloat(float32 value,
                          float32 minimum,
                          float32 maximum);

/******************************************************************************
 * Validation Utilities
 ******************************************************************************/

bool Common_IsVehicleSpeedValid(float32 speed);

bool Common_IsCameraObjectValid(const CameraObjectType *object);

bool Common_IsRadarObjectValid(const RadarObjectType *object);

bool Common_IsFusionObjectValid(const FusionObjectType *object);

/******************************************************************************
 * Initialization Utilities
 ******************************************************************************/

void Common_ResetCameraObject(CameraObjectType *object);

void Common_ResetRadarObject(RadarObjectType *object);

void Common_ResetFusionObject(FusionObjectType *object);

void Common_ResetVehicleStatus(VehicleStatusType *status);

void Common_ResetFEBOutput(FEBOutputType *output);

/******************************************************************************
 * Copy Utilities
 ******************************************************************************/

void Common_CopyVehicleStatus(VehicleStatusType *destination,
                              const VehicleStatusType *source);

void Common_CopyFEBOutput(FEBOutputType *destination,
                          const FEBOutputType *source);

/******************************************************************************
 * Alive Counter Utilities
 ******************************************************************************/

uint8 Common_UpdateAliveCounter(uint8 counter);

/******************************************************************************
 * Diagnostic Utilities
 ******************************************************************************/

bool Common_IsSignalTimeout(uint32 currentTime,
                            uint32 previousTime,
                            uint32 timeout);

void Common_ClearDiagnosticInfo(DiagnosticInfoType *diagnostic);

/******************************************************************************
 * Logging Helpers
 ******************************************************************************/

void Common_LogInfo(const char *message);

void Common_LogWarning(const char *message);

void Common_LogError(const char *message);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */