#ifndef RISK_H
#define RISK_H

int CalculateRelativeSpeed(int vehicleSpeed);

int CalculateTimeToCollision(int distance, int relativeSpeed);

int CalculateRiskScore(int distance, int ttc);

#endif