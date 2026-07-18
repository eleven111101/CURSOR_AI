/******************************************************************************
 * @file    camera_if.c
 * @brief   Camera Sensor Interface Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "camera_if.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static CameraFrameType gCameraFrame;
static bool gFrameValid = false;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void CameraIF_Init(void)
{
    memset(&gCameraFrame, 0, sizeof(gCameraFrame));
    gFrameValid = false;
}

void CameraIF_MainFunction(void)
{
    /* Placeholder for future interface diagnostics or timeout monitoring */
}

Std_ReturnType CameraIF_UpdateFrame(
    const CameraFrameType *frame)
{
    if (frame == NULL)
    {
        return E_NOT_OK;
    }

    if (frame->objectCount > CAMERA_MAX_OBJECTS)
    {
        return E_NOT_OK;
    }

    memcpy(&gCameraFrame,
           frame,
           sizeof(CameraFrameType));

    gFrameValid = true;

    return E_OK;
}

Std_ReturnType CameraIF_GetFrame(
    CameraFrameType *frame)
{
    if ((frame == NULL) || (!gFrameValid))
    {
        return E_NOT_OK;
    }

    memcpy(frame,
           &gCameraFrame,
           sizeof(CameraFrameType));

    return E_OK;
}

bool CameraIF_IsFrameValid(void)
{
    return gFrameValid;
}

void CameraIF_Reset(void)
{
    memset(&gCameraFrame, 0, sizeof(gCameraFrame));
    gFrameValid = false;
}