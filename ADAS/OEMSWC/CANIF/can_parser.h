/******************************************************************************
 * @file    can_parser.h
 * @brief   CAN Message Parser Interface
 *
 * This module decodes received CAN frames into application-level vehicle
 * signals. The decoded signals are forwarded to the Vehicle Status module,
 * which acts as the central repository of vehicle state information.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef CAN_PARSER_H
#define CAN_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "can_if.h"
#include "types.h"

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief Decoded vehicle signals extracted from CAN messages.
 */
typedef struct
{
    float32 vehicleSpeedKph;

    float32 steeringAngleDeg;

    uint8 brakePedalPressed;

    uint8 acceleratorPedalPosition;

    GearPositionType gearPosition;

    VehicleMotionStateType motionState;

    uint8 ignitionOn;

    uint8 absActive;

    uint8 espActive;

    uint8 driverDoorOpen;

    uint8 seatBeltFastened;

    uint8 turnIndicator;

    uint8 wiperStatus;

} DecodedVehicleSignalsType;

/******************************************************************************
 * Parser Statistics
 ******************************************************************************/

typedef struct
{
    uint32 parsedMessages;

    uint32 unknownMessages;

    uint32 invalidMessages;

} CanParserStatisticsType;

/******************************************************************************
 * API
 ******************************************************************************/

/**
 * @brief Initialize CAN parser.
 */
void CANParser_Init(void);

/**
 * @brief Parse a received CAN frame.
 *
 * @param frame Pointer to received CAN frame.
 *
 * @return E_OK if successfully decoded.
 */
Std_ReturnType CANParser_ParseFrame(const CanFrameType *frame);

/**
 * @brief Get the latest decoded vehicle signals.
 *
 * @return Pointer to decoded signals.
 */
const DecodedVehicleSignalsType*
CANParser_GetDecodedSignals(void);

/**
 * @brief Get parser statistics.
 *
 * @return Pointer to statistics.
 */
const CanParserStatisticsType*
CANParser_GetStatistics(void);

/**
 * @brief Reset parser statistics.
 */
void CANParser_ResetStatistics(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_PARSER_H */