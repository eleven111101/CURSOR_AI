/******************************************************************************
 * @file    vehstatus_in.h
 * @brief   Vehicle Status Input Interface
 *
 * This module receives decoded vehicle signals from the CAN Parser and updates
 * the centralized vehicle status database used by all ADAS software
 * components.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef VEHSTATUS_IN_H
#define VEHSTATUS_IN_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "can_parser.h"

/******************************************************************************
 * Vehicle Status Database
 ******************************************************************************/

typedef struct
{
    VehicleStatusType vehicleStatus;

    uint32 lastUpdateTime;

    uint32 updateCounter;

    bool dataValid;

} VehicleStatusDatabaseType;

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize Vehicle Status database.
 */
void VehStatus_Init(void);

/**
 * @brief Update vehicle status from decoded CAN signals.
 *
 * @param signals Pointer to decoded CAN signals.
 */
void VehStatus_UpdateSignals(const DecodedVehicleSignalsType *signals);

/**
 * @brief Periodic vehicle status processing.
 */
void VehStatus_MainFunction(void);

/**
 * @brief Get complete vehicle status database.
 *
 * @return Pointer to database.
 */
const VehicleStatusDatabaseType*
VehStatus_GetDatabase(void);

/**
 * @brief Check if vehicle status is valid.
 *
 * @return true if valid.
 */
bool VehStatus_IsValid(void);

/**
 * @brief Reset vehicle status database.
 */
void VehStatus_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* VEHSTATUS_IN_H */