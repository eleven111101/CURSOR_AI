/******************************************************************************
 * @file    supplier_feb.h
 * @brief   Supplier Forward Emergency Braking Interface
 *
 * Simulates a third-party supplier implementation of the Forward Emergency
 * Braking (FEB) algorithm. The OEM SWCFEB module interacts with this interface
 * instead of directly implementing supplier-specific logic.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef SUPPLIER_FEB_H
#define SUPPLIER_FEB_H

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
 * @brief Initialize supplier FEB module.
 */
void SupplierFEB_Init(void);

/**
 * @brief Execute one supplier FEB processing cycle.
 *
 * @param fusionObject Pointer to fused object.
 * @param output Pointer to FEB output.
 *
 * @return E_OK if processing succeeds.
 */
Std_ReturnType SupplierFEB_Process(
    const FusionObjectType *fusionObject,
    FEBOutputType *output);

/**
 * @brief Reset supplier internal state.
 */
void SupplierFEB_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SUPPLIER_FEB_H */