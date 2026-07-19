#include "swc2.h"
#include "common.h"
#include "risk.h"
#include "warning.h"
#include "brake.h"

static void StoreFEBOutput(int risk,
                           int warning,
                           int brake)
{
    febData.riskScore = risk;
    febData.warningRequest = warning;
    febData.brakeRequest = brake;
}

void SWC2_Run(void)
{
    int relativeSpeed;
    int ttc;
    int risk;
    int warning;
    int brake;

    relativeSpeed =
        CalculateRelativeSpeed(sensorData.vehicleSpeed);

    sensorData.relativeSpeed = relativeSpeed;

    ttc =
        CalculateTimeToCollision(sensorData.objectDistance,
                                 relativeSpeed);

    sensorData.timeToCollision = ttc;

    risk =
        CalculateRiskScore(sensorData.objectDistance,
                           ttc);

    warning =
        EvaluateWarning(risk);

    brake =
        EvaluateBrake(risk);

    StoreFEBOutput(risk,
                   warning,
                   brake);
}