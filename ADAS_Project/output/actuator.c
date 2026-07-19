#include <stdio.h>
#include "actuator.h"
#include "common.h"

void ApplyBrake(void)
{
    if(febData.brakeRequest)
    {
        printf("\nBrake Actuator : APPLIED\n");
    }
    else
    {
        printf("\nBrake Actuator : RELEASED\n");
    }
}