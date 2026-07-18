/******************************************************************************
 * @file    object_detection.h
 * @brief   Camera Object Detection Interface
 *
 * Detects and classifies objects from processed camera data. The resulting
 * object list is consumed by the Fusion SWC.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef OBJECT_DETECTION_H
#define OBJECT_DETECTION_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "camera_processing.h"

/******************************************************************************
 * Macros
 ******************************************************************************/

#define OBJECT_DETECTION_MAX_OBJECTS    CAMERA_OBJECT_LIST_SIZE

/******************************************************************************
 * Object Classification
 ******************************************************************************/

typedef enum
{
    OBJECT_UNKNOWN = 0,
    OBJECT_VEHICLE,
    OBJECT_PEDESTRIAN,
    OBJECT_CYCLIST,
    OBJECT_STATIC

} ObjectClassificationType;

/******************************************************************************
 * Detected Object
 ******************************************************************************/

typedef struct
{
    CameraObjectType cameraObject;

    ObjectClassificationType classification;

    bool tracked;

} DetectedObjectType;

/******************************************************************************
 * Object List
 ******************************************************************************/

typedef struct
{
    uint8 objectCount;

    DetectedObjectType objects[OBJECT_DETECTION_MAX_OBJECTS];

} DetectedObjectListType;

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize object detection.
 */
void ObjectDetection_Init(void);

/**
 * @brief Execute one object detection cycle.
 */
void ObjectDetection_MainFunction(void);

/**
 * @brief Detect and classify camera objects.
 *
 * @return E_OK if detection succeeds.
 */
Std_ReturnType ObjectDetection_Process(void);

/**
 * @brief Retrieve detected object list.
 *
 * @param objectList Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType ObjectDetection_GetObjects(
    DetectedObjectListType *objectList);

/**
 * @brief Reset object detection.
 */
void ObjectDetection_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* OBJECT_DETECTION_H */