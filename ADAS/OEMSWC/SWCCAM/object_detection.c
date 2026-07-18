/******************************************************************************
 * @file    object_detection.c
 * @brief   Camera Object Detection Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "object_detection.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static DetectedObjectListType gDetectedObjects;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void ObjectDetection_Init(void)
{
    memset(&gDetectedObjects, 0, sizeof(gDetectedObjects));
}

void ObjectDetection_MainFunction(void)
{
    (void)ObjectDetection_Process();
}

Std_ReturnType ObjectDetection_Process(void)
{
    CameraObjectListType cameraObjects;
    uint8 index;

    if (CameraProcessing_GetObjects(&cameraObjects) != E_OK)
    {
        return E_NOT_OK;
    }

    memset(&gDetectedObjects, 0, sizeof(gDetectedObjects));

    gDetectedObjects.objectCount = cameraObjects.objectCount;

    for (index = 0U;
         (index < cameraObjects.objectCount) &&
         (index < OBJECT_DETECTION_MAX_OBJECTS);
         index++)
    {
        gDetectedObjects.objects[index].cameraObject =
            cameraObjects.objects[index];

        gDetectedObjects.objects[index].tracked =
            (cameraObjects.objects[index].quality != SIGNAL_QUALITY_INVALID);

        if (cameraObjects.objects[index].confidence >= 0.90F)
        {
            gDetectedObjects.objects[index].classification =
                OBJECT_VEHICLE;
        }
        else if (cameraObjects.objects[index].confidence >= 0.70F)
        {
            gDetectedObjects.objects[index].classification =
                OBJECT_PEDESTRIAN;
        }
        else if (cameraObjects.objects[index].confidence >= 0.50F)
        {
            gDetectedObjects.objects[index].classification =
                OBJECT_CYCLIST;
        }
        else
        {
            gDetectedObjects.objects[index].classification =
                OBJECT_UNKNOWN;
        }
    }

    return E_OK;
}

Std_ReturnType ObjectDetection_GetObjects(
    DetectedObjectListType *objectList)
{
    if (objectList == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(objectList,
           &gDetectedObjects,
           sizeof(DetectedObjectListType));

    return E_OK;
}

void ObjectDetection_Reset(void)
{
    memset(&gDetectedObjects, 0, sizeof(gDetectedObjects));
}