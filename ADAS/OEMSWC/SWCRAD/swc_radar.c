/******************************************************************************
 * @file    swc_radar.c
 * @brief   Radar Software Component Implementation
 *
 * Receives validated radar detections from the Radar Validation module and
 * publishes processed radar objects for Sensor Fusion.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "swc_radar.h"
#include "radar_validation.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static RadarObjectListType gRadarObjects;
static bool gRadarOutputValid = false;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void SWCRadar_Init(void)
{
    memset(&gRadarObjects, 0, sizeof(gRadarObjects));
    gRadarOutputValid = false;
}

void SWCRadar_MainFunction(void)
{
    (void)SWCRadar_Process();
}

Std_ReturnType SWCRadar_Process(void)
{
    RadarObjectListType validatedObjects;

    if (RadarValidation_GetObjects(&validatedObjects) != E_OK)
    {
        gRadarOutputValid = false;
        return E_NOT_OK;
    }

    memcpy(&gRadarObjects,
           &validatedObjects,
           sizeof(RadarObjectListType));

    gRadarOutputValid = true;

    return E_OK;
}

Std_ReturnType SWCRadar_GetObjects(
    RadarObjectListType *objectList)
{
    if ((objectList == NULL) || (!gRadarOutputValid))
    {
        return E_NOT_OK;
    }

    memcpy(objectList,
           &gRadarObjects,
           sizeof(RadarObjectListType));

    return E_OK;
}

bool SWCRadar_IsValid(void)
{
    return gRadarOutputValid;
}

void SWCRadar_Reset(void)
{
    memset(&gRadarObjects, 0, sizeof(gRadarObjects));
    gRadarOutputValid = false;
}