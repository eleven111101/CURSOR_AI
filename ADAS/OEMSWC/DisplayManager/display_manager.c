/******************************************************************************
 * @file    display_manager.c
 * @brief   Display Manager Implementation
 *
 * Receives processed HMI output and generates the final display information
 * for the instrument cluster or Head-Up Display (HUD).
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "display_manager.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static DisplayOutputType gDisplayOutput;
static bool gDisplayOutputValid = false;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void DisplayManager_Init(void)
{
    memset(&gDisplayOutput, 0, sizeof(gDisplayOutput));
    gDisplayOutputValid = false;
}

void DisplayManager_MainFunction(void)
{
    (void)DisplayManager_Process();
}

Std_ReturnType DisplayManager_Process(void)
{
    HMIOutputType hmiOutput;

    if (SWCHMI_GetOutput(&hmiOutput) != E_OK)
    {
        gDisplayOutputValid = false;
        return E_NOT_OK;
    }

    gDisplayOutput.warningStatus = hmiOutput.warningStatus;
    gDisplayOutput.brakeRequest  = hmiOutput.brakeRequest;

    gDisplayOutput.showWarningIcon =
        (hmiOutput.warningStatus != WARNING_NONE);

    gDisplayOutput.showBrakeIcon =
        (hmiOutput.brakeRequest != BRAKE_REQUEST_INACTIVE);

    gDisplayOutput.showTextMessage =
        hmiOutput.displayMessage;

    gDisplayOutput.buzzerRequest =
        hmiOutput.buzzerActive;

    gDisplayOutputValid = true;

    return E_OK;
}

Std_ReturnType DisplayManager_GetOutput(
    DisplayOutputType *output)
{
    if ((output == NULL) || (!gDisplayOutputValid))
    {
        return E_NOT_OK;
    }

    memcpy(output,
           &gDisplayOutput,
           sizeof(DisplayOutputType));

    return E_OK;
}

bool DisplayManager_IsValid(void)
{
    return gDisplayOutputValid;
}

void DisplayManager_Reset(void)
{
    memset(&gDisplayOutput, 0, sizeof(gDisplayOutput));
    gDisplayOutputValid = false;
}