/******************************************************************************
 * @file    feb_output.c
 * @brief   Forward Emergency Braking Output Implementation
 *
 * Stores and publishes the latest FEB output for downstream modules such as
 * HMI, Display Manager and Diagnostics.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

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

void FEBOutput_Init(void)
{
    memset(&gFEBOutput, 0, sizeof(gFEBOutput));

    gFEBOutput.systemState = SYSTEM_NOT_INITIALIZED;
    gFEBOutput.warningStatus = WARNING_NONE;
    gFEBOutput.brakeRequest = BRAKE_REQUEST_INACTIVE;

    gFEBOutputValid = false;
}

void FEBOutput_MainFunction(void)
{
    /* Reserved for future diagnostics/logging */
}

Std_ReturnType FEBOutput_Update(
    const FEBOutputType *output)
{
    if (output == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(&gFEBOutput,
           output,
           sizeof(FEBOutputType));

    gFEBOutputValid = true;

    return E_OK;
}

Std_ReturnType FEBOutput_GetOutput(
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

bool FEBOutput_IsValid(void)
{
    return gFEBOutputValid;
}

void FEBOutput_Reset(void)
{
    memset(&gFEBOutput, 0, sizeof(gFEBOutput));

    gFEBOutput.systemState = SYSTEM_NOT_INITIALIZED;
    gFEBOutput.warningStatus = WARNING_NONE;
    gFEBOutput.brakeRequest = BRAKE_REQUEST_INACTIVE;

    gFEBOutputValid = false;
}