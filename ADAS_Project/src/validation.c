#include "validation.h"
#include "config.h"

int ValidateSpeed(int speed)
{
    if(speed < MIN_SPEED)
        return 0;

    if(speed > MAX_SPEED)
        return 0;

    return 1;
}

int ValidateDistance(int distance)
{
    if(distance < MIN_DISTANCE)
        return 0;

    if(distance > MAX_DISTANCE)
        return 0;

    return 1;
}

int ValidateInputs(int speed,int distance)
{
    if(!ValidateSpeed(speed))
        return 0;

    if(!ValidateDistance(distance))
        return 0;

    return 1;
}