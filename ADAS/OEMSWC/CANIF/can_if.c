/******************************************************************************
 * @file    can_if.c
 * @brief   CAN Interface Implementation
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "can_if.h"

#include "can_parser.h"
#include "common.h"

#include <string.h>

/******************************************************************************
 * Local Data
 ******************************************************************************/

static CanFrameType gRxQueue[CAN_RX_QUEUE_SIZE];

static uint16 gRxWriteIndex = 0U;
static uint16 gRxReadIndex  = 0U;
static uint16 gQueuedFrames = 0U;

static CanStatisticsType gStatistics;

/******************************************************************************
 * Local Function Prototypes
 ******************************************************************************/

static bool CANIF_IsFrameValid(const CanFrameType *frame);

static bool CANIF_IsQueueFull(void);

static bool CANIF_IsQueueEmpty(void);

static Std_ReturnType CANIF_PushFrame(const CanFrameType *frame);

static Std_ReturnType CANIF_PopFrame(CanFrameType *frame);

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void CANIF_Init(void)
{
    memset(gRxQueue, 0, sizeof(gRxQueue));
    memset(&gStatistics, 0, sizeof(gStatistics));

    gRxWriteIndex = 0U;
    gRxReadIndex  = 0U;
    gQueuedFrames = 0U;
}

void CANIF_MainFunction(void)
{
    CANIF_ProcessRxQueue();
}

Std_ReturnType CANIF_ReceiveFrame(const CanFrameType *frame)
{
    if(frame == NULL_PTR)
    {
        gStatistics.invalidFrames++;
        return E_NOT_OK;
    }

    if(!CANIF_IsFrameValid(frame))
    {
        gStatistics.invalidFrames++;
        return E_NOT_OK;
    }

    gStatistics.receivedFrames++;

    return CANIF_PushFrame(frame);
}

void CANIF_ProcessRxQueue(void)
{
    CanFrameType frame;

    while(!CANIF_IsQueueEmpty())
    {
        if(CANIF_PopFrame(&frame) == E_OK)
        {
            if(CANParser_ParseFrame(&frame) == E_OK)
            {
                gStatistics.parsedFrames++;
            }
            else
            {
                gStatistics.invalidFrames++;
            }
        }
    }
}

const CanStatisticsType* CANIF_GetStatistics(void)
{
    return &gStatistics;
}

void CANIF_ResetStatistics(void)
{
    memset(&gStatistics, 0, sizeof(gStatistics));
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

static bool CANIF_IsFrameValid(const CanFrameType *frame)
{
    if(frame == NULL_PTR)
    {
        return false;
    }

    if(frame->dlc > CAN_MAX_DLC)
    {
        return false;
    }

    return true;
}

static bool CANIF_IsQueueFull(void)
{
    return (gQueuedFrames >= CAN_RX_QUEUE_SIZE);
}

static bool CANIF_IsQueueEmpty(void)
{
    return (gQueuedFrames == 0U);
}

static Std_ReturnType CANIF_PushFrame(const CanFrameType *frame)
{
    if(CANIF_IsQueueFull())
    {
        gStatistics.invalidFrames++;
        return E_NOT_OK;
    }

    memcpy(&gRxQueue[gRxWriteIndex],
           frame,
           sizeof(CanFrameType));

    gRxWriteIndex++;

    if(gRxWriteIndex >= CAN_RX_QUEUE_SIZE)
    {
        gRxWriteIndex = 0U;
    }

    gQueuedFrames++;

    return E_OK;
}

static Std_ReturnType CANIF_PopFrame(CanFrameType *frame)
{
    if(CANIF_IsQueueEmpty())
    {
        return E_NOT_OK;
    }

    memcpy(frame,
           &gRxQueue[gRxReadIndex],
           sizeof(CanFrameType));

    gRxReadIndex++;

    if(gRxReadIndex >= CAN_RX_QUEUE_SIZE)
    {
        gRxReadIndex = 0U;
    }

    gQueuedFrames--;

    return E_OK;
}