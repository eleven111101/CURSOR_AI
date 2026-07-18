/******************************************************************************
 * @file    feb_output.h
 * @brief   Forward Emergency Braking Output Interface
 *
 * Responsible for publishing the final FEB decision to downstream modules
 * such as HMI, Display Manager and Diagnostics.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef FEB_OUTPUT_H
#define FEB_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize FEB output module.
 */
void FEBOutput_Init(void);

/**
 * @brief Execute one FEB output cycle.
 */
void FEBOutput_MainFunction(void);

/**
 * @brief Update the latest FEB output.
 *
 * @param output Pointer to FEB output.
 *
 * @return E_OK if successful.
 */
Std_ReturnType FEBOutput_Update(
    const FEBOutputType *output);

/**
 * @brief Retrieve the latest FEB output.
 *
 * @param output Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType FEBOutput_GetOutput(
    FEBOutputType *output);

/**
 * @brief Check whether a valid FEB output is available.
 *
 * @return true if valid.
 */
bool FEBOutput_IsValid(void);

/**
 * @brief Reset FEB output module.
 */
void FEBOutput_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* FEB_OUTPUT_H */