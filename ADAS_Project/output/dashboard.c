#include <stdio.h>
#include "dashboard.h"
#include "common.h"

void UpdateDashboard(void)
{
    printf("\n========== DASHBOARD ==========\n");
    printf("Speed              : %d km/h\n", sensorData.vehicleSpeed);
    printf("Distance           : %d m\n", sensorData.objectDistance);
    printf("Relative Speed     : %d km/h\n", sensorData.relativeSpeed);
    printf("Time To Collision  : %d s\n", sensorData.timeToCollision);

    printf("\n----- FEB -----\n");
    printf("Risk Score         : %d\n", febData.riskScore);
    printf("Warning Request    : %d\n", febData.warningRequest);
    printf("Brake Request      : %d\n", febData.brakeRequest);

    printf("\n----- HMI -----\n");
    printf("Warning Lamp       : %d\n", hmiData.warningLamp);
    printf("Buzzer             : %d\n", hmiData.buzzer);
    printf("Display State      : %d\n", hmiData.displayState);

    printf("===============================\n");
}