/******************************************************************************
 * @file    radar_validation.c
 * @brief   Radar Validation Implementation
 *
 * Validates raw radar detections by checking range, relative speed and signal
 * quality before publishing them to the Radar SWC.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "radar_validation.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static RadarObjectListType gValidatedObjects;

/******************************************************************************
 * Local Function Prototypes
 ******************************************************************************/

static bool RadarValidation_IsObjectValid(
    const RadarObjectType *object);

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void RadarValidation_Init(void)
{
    memset(&gValidatedObjects, 0, sizeof(gValidatedObjects));
}

void RadarValidation_MainFunction(void)
{
    (void)RadarValidation_Process();
}

Std_ReturnType RadarValidation_Process(void)
{
    uint8 index;

    memset(&gValidatedObjects, 0, sizeof(gValidatedObjects));

    /* Dummy validated radar objects for ADAS pipeline */
    gValidatedObjects.objectCount = 2U;

    gValidatedObjects.objects[0].objectId      = 1U;
    gValidatedObjects.objects[0].range         = 28.5F;
    gValidatedObjects.objects[0].relativeSpeed = -4.2F;
    gValidatedObjects.objects[0].angle         = -1.8F;
    gValidatedObjects.objects[0].quality       = SIGNAL_QUALITY_HIGH;

    gValidatedObjects.objects[1].objectId      = 2U;
    gValidatedObjects.objects[1].range         = 46.0F;
    gValidatedObjects.objects[1].relativeSpeed = -1.5F;
    gValidatedObjects.objects[1].angle         = 3.1F;
    gValidatedObjects.objects[1].quality       = SIGNAL_QUALITY_MEDIUM;

    for (index = 0U; index < gValidatedObjects.objectCount; index++)
    {
        if (!RadarValidation_IsObjectValid(
                &gValidatedObjects.objects[index]))
        {
            gValidatedObjects.objects[index].quality =
                SIGNAL_QUALITY_INVALID;
        }
    }

    return E_OK;
}

Std_ReturnType RadarValidation_GetObjects(
    RadarObjectListType *objectList)
{
    if (objectList == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(objectList,
           &gValidatedObjects,
           sizeof(RadarObjectListType));

    return E_OK;
}

void RadarValidation_Reset(void)
{
    memset(&gValidatedObjects, 0, sizeof(gValidatedObjects));
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

static bool RadarValidation_IsObjectValid(
    const RadarObjectType *object)
{
    if (object == NULL)
    {
        return false;
    }

    if ((object->range < RADAR_MIN_RANGE) ||
        (object->range > RADAR_MAX_RANGE))
    {
        return false;
    }

    if ((object->relativeSpeed < -RADAR_MAX_REL_SPEED) ||
        (object->relativeSpeed > RADAR_MAX_REL_SPEED))
    {
        return false;
    }

    return true;
}