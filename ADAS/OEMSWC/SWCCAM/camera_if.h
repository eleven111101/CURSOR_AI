/******************************************************************************
 * @file    camera_if.h
 * @brief   Camera Sensor Interface
 *
 * Provides the interface between the Camera Driver and the Camera SWC.
 * Responsible for acquiring raw camera detections and making them available
 * to the camera processing pipeline.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef CAMERA_IF_H
#define CAMERA_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/******************************************************************************
 * Macros
 ******************************************************************************/

#define CAMERA_MAX_OBJECTS    (16U)

/******************************************************************************
 * Raw Camera Detection
 ******************************************************************************/

typedef struct
{
    uint16 objectId;

    float32 posX;
    float32 posY;

    float32 velocity;

    bool detected;

} CameraRawObjectType;

/******************************************************************************
 * Camera Frame
 ******************************************************************************/

typedef struct
{
    uint32 frameCounter;

    uint32 timestamp;

    uint8 objectCount;

    CameraRawObjectType objects[CAMERA_MAX_OBJECTS];

} CameraFrameType;

/******************************************************************************
 * Camera Interface APIs
 ******************************************************************************/

/**
 * @brief Initialize camera interface.
 */
void CameraIF_Init(void);

/**
 * @brief Periodic camera interface task.
 */
void CameraIF_MainFunction(void);

/**
 * @brief Receive a new camera frame.
 *
 * @param frame Pointer to camera frame.
 *
 * @return E_OK if accepted.
 */
Std_ReturnType CameraIF_UpdateFrame(
    const CameraFrameType *frame);

/**
 * @brief Retrieve the latest camera frame.
 *
 * @param frame Destination buffer.
 *
 * @return E_OK if successful.
 */
Std_ReturnType CameraIF_GetFrame(
    CameraFrameType *frame);

/**
 * @brief Check whether a valid frame is available.
 *
 * @return true if valid.
 */
bool CameraIF_IsFrameValid(void);

/**
 * @brief Reset interface state.
 */
void CameraIF_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_IF_H */