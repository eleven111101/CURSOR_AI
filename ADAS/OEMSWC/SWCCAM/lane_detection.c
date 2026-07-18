/******************************************************************************
 * @file    lane_detection.c
 * @brief   Lane Detection Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "lane_detection.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static LaneInfoType gLaneInfo;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void LaneDetection_Init(void)
{
    memset(&gLaneInfo, 0, sizeof(gLaneInfo));

    gLaneInfo.status = LANE_NOT_DETECTED;
    gLaneInfo.quality = SIGNAL_QUALITY_INVALID;
}

void LaneDetection_MainFunction(void)
{
    (void)LaneDetection_Process();
}

Std_ReturnType LaneDetection_Process(void)
{
    CameraObjectListType objectList;

    if (CameraProcessing_GetObjects(&objectList) != E_OK)
    {
        return E_NOT_OK;
    }

    /* Simplified lane estimation */
    if (objectList.objectCount > 0U)
    {
        gLaneInfo.status = LANE_BOTH;

        gLaneInfo.leftLaneOffset  = -1.75F;
        gLaneInfo.rightLaneOffset =  1.75F;
        gLaneInfo.laneWidth       =  3.50F;

        gLaneInfo.quality = SIGNAL_QUALITY_HIGH;
    }
    else
    {
        gLaneInfo.status = LANE_NOT_DETECTED;

        gLaneInfo.leftLaneOffset  = 0.0F;
        gLaneInfo.rightLaneOffset = 0.0F;
        gLaneInfo.laneWidth       = 0.0F;

        gLaneInfo.quality = SIGNAL_QUALITY_INVALID;
    }

    return E_OK;
}

Std_ReturnType LaneDetection_GetLaneInfo(
    LaneInfoType *laneInfo)
{
    if (laneInfo == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(laneInfo,
           &gLaneInfo,
           sizeof(LaneInfoType));

    return E_OK;
}

void LaneDetection_Reset(void)
{
    memset(&gLaneInfo, 0, sizeof(gLaneInfo));

    gLaneInfo.status = LANE_NOT_DETECTED;
    gLaneInfo.quality = SIGNAL_QUALITY_INVALID;
}