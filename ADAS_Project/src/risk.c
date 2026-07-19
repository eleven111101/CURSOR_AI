#include "risk.h"

int CalculateRelativeSpeed(int vehicleSpeed)
{
    /* Assume obstacle is stationary */
    return vehicleSpeed;
}

int CalculateTimeToCollision(int distance, int relativeSpeed)
{
    if(relativeSpeed <= 0)
        return 999;

    return (distance * 10) / relativeSpeed;
}

int CalculateRiskScore(int distance, int ttc)
{
    int risk = 0;

    if(distance <= 8)
        risk += 50;
    else if(distance <= 15)
        risk += 30;

    if(ttc <= 2)
        risk += 50;
    else if(ttc <= 4)
        risk += 30;

    if(risk > 100)
        risk = 100;

    return risk;
}