/******************************************************************************
 * @file    camera_output.h
 * @brief   Camera Output Interface
 *
 * Publishes processed camera outputs for downstream ADAS components such as
 * Sensor Fusion and Diagnostics.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef CAMERA_OUTPUT_H
#define CAMERA_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "object_detection.h"
#include "lane_detection.h"

/******************************************************************************
 * Camera Output
 ******************************************************************************/

typedef struct
{
    DetectedObjectListType detectedObjects;

    LaneInfoType laneInfo;

    uint32 frameCounter;

    bool valid;

} CameraOutputType;

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize camera output module.
 */
void CameraOutput_Init(void);

/**
 * @brief Execute one camera output cycle.
 */
void CameraOutput_MainFunction(void);

/**
 * @brief Update the camera output from processing modules.
 *
 * @return E_OK if successful.
 */
Std_ReturnType CameraOutput_Update(void);

/**
 * @brief Get the latest camera output.
 *
 * @param output Pointer to destination structure.
 *
 * @return E_OK if successful.
 */
Std_ReturnType CameraOutput_GetOutput(
    CameraOutputType *output);

/**
 * @brief Check whether the camera output is valid.
 *
 * @return true if valid.
 */
bool CameraOutput_IsValid(void);

/**
 * @brief Reset camera output.
 */
void CameraOutput_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_OUTPUT_H */