/******************************************************************************
 * @file    can_parser.c
 * @brief   CAN Message Parser Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "can_parser.h"

#include "common.h"
#include "constants.h"

/* Forward declaration - implemented in VehStatus module */
extern void VehStatus_UpdateSignals(const DecodedVehicleSignalsType *signals);

/******************************************************************************
 * Local Data
 ******************************************************************************/

static DecodedVehicleSignalsType gDecodedSignals;

static CanParserStatisticsType gParserStatistics;

/******************************************************************************
 * Local Function Prototypes
 ******************************************************************************/

static Std_ReturnType CANParser_ParseVehicleSpeed(const CanFrameType *frame);
static Std_ReturnType CANParser_ParseSteeringAngle(const CanFrameType *frame);
static Std_ReturnType CANParser_ParseBrakeStatus(const CanFrameType *frame);
static Std_ReturnType CANParser_ParseGearPosition(const CanFrameType *frame);
static Std_ReturnType CANParser_ParseVehicleState(const CanFrameType *frame);

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void CANParser_Init(void)
{
    CLEAR_MEMORY(&gDecodedSignals, sizeof(gDecodedSignals));
    CLEAR_MEMORY(&gParserStatistics, sizeof(gParserStatistics));
}

Std_ReturnType CANParser_ParseFrame(const CanFrameType *frame)
{
    Std_ReturnType status = E_NOT_OK;

    if(frame == NULL_PTR)
    {
        gParserStatistics.invalidMessages++;
        return E_NOT_OK;
    }

    switch(frame->messageId)
    {
        case CAN_ID_VEHICLE_SPEED:
            status = CANParser_ParseVehicleSpeed(frame);
            break;

        case CAN_ID_STEERING_ANGLE:
            status = CANParser_ParseSteeringAngle(frame);
            break;

        case CAN_ID_BRAKE_STATUS:
            status = CANParser_ParseBrakeStatus(frame);
            break;

        case CAN_ID_GEAR_POSITION:
            status = CANParser_ParseGearPosition(frame);
            break;

        default:
            gParserStatistics.unknownMessages++;
            return E_NOT_OK;
    }

    if(status == E_OK)
    {
        gParserStatistics.parsedMessages++;

        VehStatus_UpdateSignals(&gDecodedSignals);
    }
    else
    {
        gParserStatistics.invalidMessages++;
    }

    return status;
}

const DecodedVehicleSignalsType* CANParser_GetDecodedSignals(void)
{
    return &gDecodedSignals;
}

const CanParserStatisticsType* CANParser_GetStatistics(void)
{
    return &gParserStatistics;
}

void CANParser_ResetStatistics(void)
{
    CLEAR_MEMORY(&gParserStatistics, sizeof(gParserStatistics));
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

static Std_ReturnType CANParser_ParseVehicleSpeed(const CanFrameType *frame)
{
    uint16 rawSpeed;

    rawSpeed = ((uint16)frame->data[1] << 8U) |
                (uint16)frame->data[0];

    gDecodedSignals.vehicleSpeedKph = (float32)rawSpeed * 0.01F;

    if(!Common_IsVehicleSpeedValid(gDecodedSignals.vehicleSpeedKph))
    {
        return E_NOT_OK;
    }

    if(gDecodedSignals.vehicleSpeedKph > 0.1F)
    {
        gDecodedSignals.motionState = VEHICLE_MOVING;
    }
    else
    {
        gDecodedSignals.motionState = VEHICLE_STOPPED;
    }

    return E_OK;
}

static Std_ReturnType CANParser_ParseSteeringAngle(const CanFrameType *frame)
{
    sint16 rawAngle;

    rawAngle = (sint16)(((uint16)frame->data[1] << 8U) |
                         (uint16)frame->data[0]);

    gDecodedSignals.steeringAngleDeg = (float32)rawAngle * 0.1F;

    return E_OK;
}

static Std_ReturnType CANParser_ParseBrakeStatus(const CanFrameType *frame)
{
    gDecodedSignals.brakePedalPressed = (frame->data[0] & 0x01U) ? 1U : 0U;
    gDecodedSignals.absActive         = (frame->data[0] & 0x02U) ? 1U : 0U;
    gDecodedSignals.espActive         = (frame->data[0] & 0x04U) ? 1U : 0U;

    return E_OK;
}

static Std_ReturnType CANParser_ParseGearPosition(const CanFrameType *frame)
{
    switch(frame->data[0])
    {
        case 0U:
            gDecodedSignals.gearPosition = GEAR_PARK;
            break;

        case 1U:
            gDecodedSignals.gearPosition = GEAR_REVERSE;
            break;

        case 2U:
            gDecodedSignals.gearPosition = GEAR_NEUTRAL;
            break;

        case 3U:
            gDecodedSignals.gearPosition = GEAR_DRIVE;
            break;

        default:
            return E_NOT_OK;
    }

    return E_OK;
}

static Std_ReturnType CANParser_ParseVehicleState(const CanFrameType *frame)
{
    gDecodedSignals.ignitionOn       = (frame->data[0] & 0x01U) ? 1U : 0U;
    gDecodedSignals.driverDoorOpen   = (frame->data[0] & 0x02U) ? 1U : 0U;
    gDecodedSignals.seatBeltFastened = (frame->data[0] & 0x04U) ? 1U : 0U;
    gDecodedSignals.turnIndicator    = frame->data[1];
    gDecodedSignals.wiperStatus      = frame->data[2];
    gDecodedSignals.acceleratorPedalPosition = frame->data[3];

    return E_OK;
}