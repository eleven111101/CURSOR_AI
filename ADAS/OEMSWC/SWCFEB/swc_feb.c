/******************************************************************************
 * @file    swc_feb.c
 * @brief   Forward Emergency Braking (FEB) Software Component Implementation
 *
 * Coordinates the FEB processing pipeline by obtaining fused objects,
 * executing the FEB logic and publishing the resulting warning and brake
 * requests.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "swc_feb.h"
#include "swc_fusion.h"
#include "feb_logic.h"
#include "feb_output.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static FEBOutputType gFEBOutput;
static bool gFEBOutputValid = false;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void SWCFEB_Init(void)
{
    memset(&gFEBOutput, 0, sizeof(gFEBOutput));
    gFEBOutput.systemState = SYSTEM_INITIALIZING;
    gFEBOutputValid = false;
}

void SWCFEB_MainFunction(void)
{
    (void)SWCFEB_Process();
}

Std_ReturnType SWCFEB_Process(void)
{
    FusionObjectListType fusionObjects;

    if (SWCFusion_GetObjects(&fusionObjects) != E_OK)
    {
        gFEBOutputValid = false;
        return E_NOT_OK;
    }

    if (FEBLogic_Process(&fusionObjects,
                         &gFEBOutput) != E_OK)
    {
        gFEBOutputValid = false;
        return E_NOT_OK;
    }

    if (FEBOutput_Update(&gFEBOutput) != E_OK)
    {
        gFEBOutputValid = false;
        return E_NOT_OK;
    }

    gFEBOutputValid = true;

    return E_OK;
}

Std_ReturnType SWCFEB_GetOutput(
    FEBOutputType *output)
{
    if ((output == NULL) || (!gFEBOutputValid))
    {
        return E_NOT_OK;
    }

    memcpy(output,
           &gFEBOutput,
           sizeof(FEBOutputType));

    return E_OK;
}

bool SWCFEB_IsValid(void)
{
    return gFEBOutputValid;
}

void SWCFEB_Reset(void)
{
    memset(&gFEBOutput, 0, sizeof(gFEBOutput));
    gFEBOutput.systemState = SYSTEM_INITIALIZING;
    gFEBOutputValid = false;
}