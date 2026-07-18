/******************************************************************************
 * @file calibration.h
 ******************************************************************************/

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "types.h"

typedef struct
{
    float32 warningTTC;

    float32 brakeTTC;

    float32 collisionProbabilityThreshold;

    float32 minimumVehicleSpeed;

    float32 maximumVehicleSpeed;

} FEBCalibrationType;

void Calibration_Init(void);

const FEBCalibrationType*
Calibration_GetFEBCalibration(void);

#endif