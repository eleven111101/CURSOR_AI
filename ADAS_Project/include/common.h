#ifndef COMMON_H
#define COMMON_H

typedef struct
{
    int vehicleSpeed;
    int objectDistance;
    int relativeSpeed;
    int timeToCollision;
} SensorData;

typedef struct
{
    int riskScore;
    int warningRequest;
    int brakeRequest;
} FEBData;

typedef struct
{
    int warningLamp;
    int buzzer;
    int displayState;
} HMIData;

extern SensorData sensorData;
extern FEBData febData;
extern HMIData hmiData;

#endif