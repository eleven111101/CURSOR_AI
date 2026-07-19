#include "warning.h"
#include "config.h"

int EvaluateWarning(int riskScore)
{
    if(FEATURE_ENABLE == OFF)
        return OFF;

    if(riskScore >= WARNING_RISK)
        return ON;

    return OFF;
}