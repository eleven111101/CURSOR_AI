#include "swc4.h"
#include "dashboard.h"
#include "actuator.h"
#include "logger.h"

void SWC4_Run(void)
{
    UpdateDashboard();

    ApplyBrake();

    LogSystemStatus();
}