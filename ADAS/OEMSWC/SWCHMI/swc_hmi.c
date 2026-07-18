/******************************************************************************
 * @file    swc_hmi.c
 * @brief   Human Machine Interface (HMI) Software Component Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "swc_hmi.h"
#include "feb_output.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static HMIOutputType gHMIOutput;
static bool gHMIOutputValid = false;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void SWCHMI_Init(void)
{
    memset(&gHMIOutput, 0, sizeof(gHMIOutput));
    gHMIOutputValid = false;
}

void SWCHMI_MainFunction(void)
{
    (void)SWCHMI_Process();
}

Std_ReturnType SWCHMI_Process(void)
{
    FEBOutputType febOutput;

    if (FEBOutput_GetOutput(&febOutput) != E_OK)
    {
        gHMIOutputValid = false;
        return E_NOT_OK;
    }

    gHMIOutput.warningStatus = febOutput.warningStatus;
    gHMIOutput.brakeRequest  = febOutput.brakeRequest;

    gHMIOutput.buzzerActive =
        (febOutput.warningStatus != WARNING_NONE);

    gHMIOutput.warningLampOn =
        (febOutput.warningStatus == WARNING_ACTIVE);

    gHMIOutput.displayMessage =
        (febOutput.warningStatus != WARNING_NONE);

    gHMIOutputValid = true;

    return E_OK;
}

Std_ReturnType SWCHMI_GetOutput(
    HMIOutputType *output)
{
    if ((output == NULL) || (!gHMIOutputValid))
    {
        return E_NOT_OK;
    }

    memcpy(output,
           &gHMIOutput,
           sizeof(HMIOutputType));

    return E_OK;
}

bool SWCHMI_IsValid(void)
{
    return gHMIOutputValid;
}

void SWCHMI_Reset(void)
{
    memset(&gHMIOutput, 0, sizeof(gHMIOutput));
    gHMIOutputValid = false;
}