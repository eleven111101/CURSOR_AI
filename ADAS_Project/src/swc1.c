#include "swc1.h"
#include "sensor_input.h"
#include "validation.h"
#include "common.h"

static void StoreInputs(int speed,int distance)
{
    sensorData.vehicleSpeed = speed;
    sensorData.objectDistance = distance;
}

void SWC1_Run(void)
{
    int speed;
    int distance;

    speed = ReadSpeedSensor();

    distance = ReadDistanceSensor();

    if(ValidateInputs(speed,distance))
    {
        StoreInputs(speed,distance);
    }
}