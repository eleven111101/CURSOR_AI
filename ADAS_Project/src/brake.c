#include "brake.h"
#include "config.h"

int EvaluateBrake(int riskScore)
{
    if(FEATURE_ENABLE == OFF)
        return OFF;

    if(riskScore >= BRAKE_RISK)
        return ON;

    return OFF;
}