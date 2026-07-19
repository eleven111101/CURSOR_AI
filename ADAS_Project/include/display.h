#ifndef DISPLAY_H
#define DISPLAY_H

void UpdateWarningLamp(int warningRequest);

void UpdateBrakeLamp(int brakeRequest);

void UpdateBuzzer(int warningRequest, int brakeRequest);

void UpdateDisplay(int warningRequest, int brakeRequest);

void GenerateHMIStatus(void);

#endif