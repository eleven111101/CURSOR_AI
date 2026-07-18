/******************************************************************************
 * @file    can_if.h
 * @brief   CAN Interface Module
 *
 * This module provides an abstraction between the CAN driver and the ADAS
 * application software. It receives CAN frames, validates them, and forwards
 * them to the CAN parser for signal extraction.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef CAN_IF_H
#define CAN_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "constants.h"

/******************************************************************************
 * Macro Definitions
 ******************************************************************************/

#define CAN_MAX_DLC                 (8U)

#define CAN_RX_QUEUE_SIZE           (64U)

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief Standard CAN Frame
 */
typedef struct
{
    uint32 messageId;

    uint8 dlc;

    uint8 data[CAN_MAX_DLC];

    uint32 timestamp;

} CanFrameType;

/**
 * @brief CAN Interface Statistics
 */
typedef struct
{
    uint32 receivedFrames;

    uint32 parsedFrames;

    uint32 invalidFrames;

    uint32 timeoutEvents;

} CanStatisticsType;

/******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief Initialize CAN Interface.
 */
void CANIF_Init(void);

/**
 * @brief Periodic CAN Interface task.
 */
void CANIF_MainFunction(void);

/**
 * @brief Receive a CAN frame from the lower layer.
 *
 * This function is typically called by the CAN driver.
 *
 * @param frame Pointer to received frame.
 *
 * @return E_OK if accepted.
 */
Std_ReturnType CANIF_ReceiveFrame(const CanFrameType *frame);

/**
 * @brief Process all received CAN frames.
 */
void CANIF_ProcessRxQueue(void);

/**
 * @brief Retrieve current CAN statistics.
 *
 * @return Pointer to statistics structure.
 */
const CanStatisticsType*
CANIF_GetStatistics(void);

/**
 * @brief Clear interface statistics.
 */
void CANIF_ResetStatistics(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_IF_H */