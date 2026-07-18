/******************************************************************************
 * @file    swc_fusion.c
 * @brief   Sensor Fusion Software Component Implementation
 *
 * Combines Camera and Radar perception data into a unified Fusion object list.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "swc_fusion.h"
#include "camera_output.h"
#include "swc_radar.h"
#include "fusion_filter.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static FusionObjectListType gFusionObjects;
static bool gFusionValid = false;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void SWCFusion_Init(void)
{
    memset(&gFusionObjects, 0, sizeof(gFusionObjects));
    gFusionValid = false;
}

void SWCFusion_MainFunction(void)
{
    (void)SWCFusion_Process();
}

Std_ReturnType SWCFusion_Process(void)
{
    CameraOutputType cameraOutput;
    RadarObjectListType radarObjects;

    if ((CameraOutput_GetOutput(&cameraOutput) != E_OK) ||
        (SWCRadar_GetObjects(&radarObjects) != E_OK))
    {
        gFusionValid = false;
        return E_NOT_OK;
    }

    if (FusionFilter_Process(&cameraOutput,
                             &radarObjects,
                             &gFusionObjects) != E_OK)
    {
        gFusionValid = false;
        return E_NOT_OK;
    }

    gFusionValid = true;

    return E_OK;
}

Std_ReturnType SWCFusion_GetObjects(
    FusionObjectListType *objectList)
{
    if ((objectList == NULL) || (!gFusionValid))
    {
        return E_NOT_OK;
    }

    memcpy(objectList,
           &gFusionObjects,
           sizeof(FusionObjectListType));

    return E_OK;
}

bool SWCFusion_IsValid(void)
{
    return gFusionValid;
}

void SWCFusion_Reset(void)
{
    memset(&gFusionObjects, 0, sizeof(gFusionObjects));
    gFusionValid = false;
}

