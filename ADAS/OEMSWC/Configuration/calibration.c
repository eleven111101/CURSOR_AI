/******************************************************************************
 * @file calibration.c
 ******************************************************************************/

#include "calibration.h"

static FEBCalibrationType gCalibration =
{
    .warningTTC = 2.5F,
    .brakeTTC = 1.2F,
    .collisionProbabilityThreshold = 0.80F,
    .minimumVehicleSpeed = 15.0F,
    .maximumVehicleSpeed = 180.0F
};

void Calibration_Init(void)
{
}

const FEBCalibrationType*
Calibration_GetFEBCalibration(void)
{
    return &gCalibration;
}