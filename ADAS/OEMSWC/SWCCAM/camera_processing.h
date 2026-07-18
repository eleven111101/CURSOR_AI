/******************************************************************************
 * @file    camera_processing.h
 * @brief   Camera Processing Interface
 *
 * Converts raw camera detections into validated camera objects for use by
 * downstream ADAS components such as Fusion.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef CAMERA_PROCESSING_H
#define CAMERA_PROCESSING_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "camera_if.h"

/******************************************************************************
 * Macros
 ******************************************************************************/

#define CAMERA_OBJECT_LIST_SIZE    CAMERA_MAX_OBJECTS

/******************************************************************************
 * Camera Processing Output
 ******************************************************************************/

typedef struct
{
    uint8 objectCount;

    CameraObjectType objects[CAMERA_OBJECT_LIST_SIZE];

} CameraObjectListType;

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize camera processing.
 */
void CameraProcessing_Init(void);

/**
 * @brief Execute one camera processing cycle.
 */
void CameraProcessing_MainFunction(void);

/**
 * @brief Process the latest camera frame.
 *
 * @return E_OK if processing succeeds.
 */
Std_ReturnType CameraProcessing_ProcessFrame(void);

/**
 * @brief Retrieve processed camera objects.
 *
 * @param objectList Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType CameraProcessing_GetObjects(
    CameraObjectListType *objectList);

/**
 * @brief Reset camera processing.
 */
void CameraProcessing_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_PROCESSING_H */