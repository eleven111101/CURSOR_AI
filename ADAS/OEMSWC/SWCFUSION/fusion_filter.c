/******************************************************************************
 * @file    fusion_filter.c
 * @brief   Sensor Fusion Filter Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "fusion_filter.h"

#include <string.h>

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void FusionFilter_Init(void)
{
}

void FusionFilter_MainFunction(void)
{
}

Std_ReturnType FusionFilter_Process(
    const CameraOutputType *cameraOutput,
    const RadarObjectListType *radarObjects,
    FusionObjectListType *fusionObjects)
{
    uint8 index;
    uint8 fusionCount;

    if ((cameraOutput == NULL) ||
        (radarObjects == NULL) ||
        (fusionObjects == NULL))
    {
        return E_NOT_OK;
    }

    memset(fusionObjects, 0, sizeof(FusionObjectListType));

    fusionCount = cameraOutput->detectedObjects.objectCount;

    if (radarObjects->objectCount < fusionCount)
    {
        fusionCount = radarObjects->objectCount;
    }

    if (fusionCount > FUSION_MAX_OBJECTS)
    {
        fusionCount = FUSION_MAX_OBJECTS;
    }

    fusionObjects->objectCount = fusionCount;

    for (index = 0U; index < fusionCount; index++)
    {
        fusionObjects->objects[index].objectId =
            cameraOutput->detectedObjects.objects[index].cameraObject.objectId;

        fusionObjects->objects[index].distance =
            radarObjects->objects[index].range;

        fusionObjects->objects[index].relativeSpeed =
            radarObjects->objects[index].relativeSpeed;

        if (radarObjects->objects[index].relativeSpeed < -FUSION_MIN_RELATIVE_SPEED)
        {
            fusionObjects->objects[index].timeToCollision =
                radarObjects->objects[index].range /
                (-radarObjects->objects[index].relativeSpeed);
        }
        else
        {
            fusionObjects->objects[index].timeToCollision =
                FUSION_MAX_TTC;
        }

        fusionObjects->objects[index].collisionProbability =
            cameraOutput->detectedObjects.objects[index]
            .cameraObject.confidence;

        fusionObjects->objects[index].valid =
            (cameraOutput->detectedObjects.objects[index].tracked &&
             radarObjects->objects[index].quality != SIGNAL_QUALITY_INVALID);
    }

    return E_OK;
}

void FusionFilter_Reset(void)
{
}