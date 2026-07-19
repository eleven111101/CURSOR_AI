#include <stdio.h>
#include "logger.h"
#include "common.h"

void LogSystemStatus(void)
{
    printf("\n[LOG]");
    printf(" Speed=%d", sensorData.vehicleSpeed);
    printf(" Distance=%d", sensorData.objectDistance);
    printf(" Risk=%d", febData.riskScore);
    printf(" Warning=%d", febData.warningRequest);
    printf(" Brake=%d", febData.brakeRequest);
    printf("\n");
}