/******************************************************************************
 * @file    swc_feb.h
 * @brief   Forward Emergency Braking (FEB) Software Component Interface
 *
 * Coordinates the Forward Emergency Braking decision pipeline. The FEB SWC
 * receives fused objects from the Sensor Fusion module, invokes the braking
 * decision logic and publishes warning/brake requests.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef SWC_FEB_H
#define SWC_FEB_H

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
 * @brief Initialize the FEB software component.
 */
void SWCFEB_Init(void);

/**
 * @brief Execute one FEB processing cycle.
 */
void SWCFEB_MainFunction(void);

/**
 * @brief Execute the FEB decision pipeline.
 *
 * @return E_OK if processing succeeds.
 */
Std_ReturnType SWCFEB_Process(void);

/**
 * @brief Retrieve the latest FEB output.
 *
 * @param output Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType SWCFEB_GetOutput(
    FEBOutputType *output);

/**
 * @brief Check whether the FEB output is valid.
 *
 * @return true if valid.
 */
bool SWCFEB_IsValid(void);

/**
 * @brief Reset the FEB software component.
 */
void SWCFEB_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SWC_FEB_H */