/******************************************************************************
 * @file    camera_processing.c
 * @brief   Camera Processing Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "camera_processing.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static CameraObjectListType gCameraObjects;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void CameraProcessing_Init(void)
{
    memset(&gCameraObjects, 0, sizeof(gCameraObjects));
}

void CameraProcessing_MainFunction(void)
{
    (void)CameraProcessing_ProcessFrame();
}

Std_ReturnType CameraProcessing_ProcessFrame(void)
{
    CameraFrameType frame;
    uint8 index;

    if (CameraIF_GetFrame(&frame) != E_OK)
    {
        return E_NOT_OK;
    }

    memset(&gCameraObjects, 0, sizeof(gCameraObjects));

    gCameraObjects.objectCount = frame.objectCount;

    for (index = 0U;
         (index < frame.objectCount) &&
         (index < CAMERA_OBJECT_LIST_SIZE);
         index++)
    {
        gCameraObjects.objects[index].objectId =
            frame.objects[index].objectId;

        gCameraObjects.objects[index].posX =
            frame.objects[index].posX;

        gCameraObjects.objects[index].posY =
            frame.objects[index].posY;

        gCameraObjects.objects[index].velocity =
            frame.objects[index].velocity;

        gCameraObjects.objects[index].confidence =
            frame.objects[index].detected ? 1.0F : 0.0F;

        gCameraObjects.objects[index].quality =
            frame.objects[index].detected ?
            SIGNAL_QUALITY_HIGH :
            SIGNAL_QUALITY_INVALID;
    }

    return E_OK;
}

Std_ReturnType CameraProcessing_GetObjects(
    CameraObjectListType *objectList)
{
    if (objectList == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(objectList,
           &gCameraObjects,
           sizeof(CameraObjectListType));

    return E_OK;
}

void CameraProcessing_Reset(void)
{
    memset(&gCameraObjects, 0, sizeof(gCameraObjects));
}