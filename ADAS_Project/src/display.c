#include "display.h"
#include "common.h"
#include "config.h"

void UpdateWarningLamp(int warningRequest)
{
    if(warningRequest == ON)
        hmiData.warningLamp = ON;
    else
        hmiData.warningLamp = OFF;
}

void UpdateBrakeLamp(int brakeRequest)
{
    /* Reserved for future enhancement */
    (void)brakeRequest;
}

void UpdateBuzzer(int warningRequest, int brakeRequest)
{
    if((warningRequest == ON) || (brakeRequest == ON))
        hmiData.buzzer = ON;
    else
        hmiData.buzzer = OFF;
}

void UpdateDisplay(int warningRequest, int brakeRequest)
{
    if(brakeRequest == ON)
    {
        hmiData.displayState = DISPLAY_BRAKE;
    }
    else if(warningRequest == ON)
    {
        hmiData.displayState = DISPLAY_WARNING;
    }
    else
    {
        hmiData.displayState = DISPLAY_NORMAL;
    }
}

void GenerateHMIStatus(void)
{
    /* Placeholder for future diagnostics/logging */
}