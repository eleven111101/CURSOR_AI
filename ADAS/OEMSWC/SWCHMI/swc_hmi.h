/******************************************************************************
 * @file    swc_hmi.h
 * @brief   Human Machine Interface (HMI) Software Component Interface
 *
 * Receives the output of the FEB module and converts it into driver-facing
 * warning indications for the Display Manager.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef SWC_HMI_H
#define SWC_HMI_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/******************************************************************************
 * HMI Output
 ******************************************************************************/

typedef struct
{
    WarningStatusType warningStatus;

    BrakeRequestType brakeRequest;

    bool buzzerActive;

    bool warningLampOn;

    bool displayMessage;

} HMIOutputType;

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize HMI SWC.
 */
void SWCHMI_Init(void);

/**
 * @brief Execute one HMI processing cycle.
 */
void SWCHMI_MainFunction(void);

/**
 * @brief Process FEB output into driver warnings.
 *
 * @return E_OK if processing succeeds.
 */
Std_ReturnType SWCHMI_Process(void);

/**
 * @brief Retrieve HMI output.
 *
 * @param output Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType SWCHMI_GetOutput(
    HMIOutputType *output);

/**
 * @brief Check whether HMI output is valid.
 *
 * @return true if valid.
 */
bool SWCHMI_IsValid(void);

/**
 * @brief Reset HMI SWC.
 */
void SWCHMI_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SWC_HMI_H */