/******************************************************************************
 * @file    display_manager.h
 * @brief   Display Manager Interface
 *
 * Receives HMI output and converts it into dashboard display information.
 * This module acts as the interface between the ADAS software stack and the
 * instrument cluster/HUD.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "swc_hmi.h"

/******************************************************************************
 * Display Output
 ******************************************************************************/

typedef struct
{
    WarningStatusType warningStatus;

    BrakeRequestType brakeRequest;

    bool showWarningIcon;

    bool showBrakeIcon;

    bool showTextMessage;

    bool buzzerRequest;

} DisplayOutputType;

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize Display Manager.
 */
void DisplayManager_Init(void);

/**
 * @brief Execute one Display Manager cycle.
 */
void DisplayManager_MainFunction(void);

/**
 * @brief Process HMI output and generate display information.
 *
 * @return E_OK if successful.
 */
Std_ReturnType DisplayManager_Process(void);

/**
 * @brief Retrieve the current display output.
 *
 * @param output Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType DisplayManager_GetOutput(
    DisplayOutputType *output);

/**
 * @brief Check whether display output is valid.
 *
 * @return true if valid.
 */
bool DisplayManager_IsValid(void);

/**
 * @brief Reset Display Manager.
 */
void DisplayManager_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_MANAGER_H */