/******************************************************************************
 * @file    lane_detection.h
 * @brief   Lane Detection Interface
 *
 * Detects lane boundaries from processed camera objects. The detected lane
 * information is used by downstream ADAS functions and may later be extended
 * for Lane Departure Warning (LDW) and Lane Keeping Assist (LKA).
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef LANE_DETECTION_H
#define LANE_DETECTION_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "camera_processing.h"

/******************************************************************************
 * Lane Definitions
 ******************************************************************************/

typedef enum
{
    LANE_NOT_DETECTED = 0,
    LANE_LEFT,
    LANE_RIGHT,
    LANE_BOTH

} LaneDetectionStatusType;

/******************************************************************************
 * Lane Information
 ******************************************************************************/

typedef struct
{
    LaneDetectionStatusType status;

    float32 leftLaneOffset;
    float32 rightLaneOffset;

    float32 laneWidth;

    SignalQualityType quality;

} LaneInfoType;

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize lane detection.
 */
void LaneDetection_Init(void);

/**
 * @brief Execute one lane detection cycle.
 */
void LaneDetection_MainFunction(void);

/**
 * @brief Detect lanes from processed camera objects.
 *
 * @return E_OK if detection succeeds.
 */
Std_ReturnType LaneDetection_Process(void);

/**
 * @brief Retrieve detected lane information.
 *
 * @param laneInfo Pointer to destination structure.
 *
 * @return E_OK if successful.
 */
Std_ReturnType LaneDetection_GetLaneInfo(
    LaneInfoType *laneInfo);

/**
 * @brief Reset lane detection state.
 */
void LaneDetection_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* LANE_DETECTION_H */