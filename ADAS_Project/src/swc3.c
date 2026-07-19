#include "swc3.h"
#include "common.h"
#include "display.h"

void SWC3_Run(void)
{
    UpdateWarningLamp(febData.warningRequest);

    UpdateBrakeLamp(febData.brakeRequest);

    UpdateBuzzer(febData.warningRequest,
                 febData.brakeRequest);

    UpdateDisplay(febData.warningRequest,
                  febData.brakeRequest);

    GenerateHMIStatus();
}