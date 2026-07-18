/******************************************************************************
 * @file    camera_output.c
 * @brief   Camera Output Implementation
 *
 * Aggregates outputs from Object Detection and Lane Detection into a single
 * camera output package for downstream modules such as Sensor Fusion.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "camera_output.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static CameraOutputType gCameraOutput;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void CameraOutput_Init(void)
{
    memset(&gCameraOutput, 0, sizeof(gCameraOutput));

    gCameraOutput.valid = false;
}

void CameraOutput_MainFunction(void)
{
    (void)CameraOutput_Update();
}

Std_ReturnType CameraOutput_Update(void)
{
    static uint32 frameCounter = 0U;

    if (ObjectDetection_GetObjects(
            &gCameraOutput.detectedObjects) != E_OK)
    {
        return E_NOT_OK;
    }

    if (LaneDetection_GetLaneInfo(
            &gCameraOutput.laneInfo) != E_OK)
    {
        return E_NOT_OK;
    }

    frameCounter++;
    gCameraOutput.frameCounter = frameCounter;
    gCameraOutput.valid = true;

    return E_OK;
}

Std_ReturnType CameraOutput_GetOutput(
    CameraOutputType *output)
{
    if (output == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(output,
           &gCameraOutput,
           sizeof(CameraOutputType));

    return E_OK;
}

bool CameraOutput_IsValid(void)
{
    return gCameraOutput.valid;
}

void CameraOutput_Reset(void)
{
    memset(&gCameraOutput, 0, sizeof(gCameraOutput));

    gCameraOutput.valid = false;
}