/******************************************************************************
 * @file    feb_logic.h
 * @brief   Forward Emergency Braking Decision Logic Interface
 *
 * Implements the decision-making algorithm for the Forward Emergency Braking
 * system. The module evaluates fused objects, vehicle state and calibration
 * parameters to determine warning and braking requests.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef FEB_LOGIC_H
#define FEB_LOGIC_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "swc_fusion.h"

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize FEB decision logic.
 */
void FEBLogic_Init(void);

/**
 * @brief Execute one FEB logic cycle.
 */
void FEBLogic_MainFunction(void);

/**
 * @brief Evaluate fused objects and generate FEB output.
 *
 * @param fusionObjects Pointer to fused object list.
 * @param output Pointer to FEB output.
 *
 * @return E_OK if processing succeeds.
 */
Std_ReturnType FEBLogic_Process(
    const FusionObjectListType *fusionObjects,
    FEBOutputType *output);

/**
 * @brief Reset FEB logic.
 */
void FEBLogic_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* FEB_LOGIC_H */