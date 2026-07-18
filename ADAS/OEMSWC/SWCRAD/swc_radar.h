/******************************************************************************
 * @file    swc_radar.h
 * @brief   Radar Software Component Interface
 *
 * Receives validated radar detections, performs radar processing and provides
 * radar objects to the Sensor Fusion module.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef SWC_RADAR_H
#define SWC_RADAR_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/******************************************************************************
 * Macros
 ******************************************************************************/

#define RADAR_MAX_OBJECTS    (16U)

/******************************************************************************
 * Radar Object List
 ******************************************************************************/

typedef struct
{
    uint8 objectCount;

    RadarObjectType objects[RADAR_MAX_OBJECTS];

} RadarObjectListType;

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize Radar SWC.
 */
void SWCRadar_Init(void);

/**
 * @brief Execute one radar processing cycle.
 */
void SWCRadar_MainFunction(void);

/**
 * @brief Process radar detections.
 *
 * This function acquires validated radar data, updates the processed radar
 * object list and makes it available for Sensor Fusion.
 *
 * @return E_OK if processing succeeds.
 */
Std_ReturnType SWCRadar_Process(void);

/**
 * @brief Get processed radar objects.
 *
 * @param objectList Pointer to destination object list.
 *
 * @return E_OK if successful.
 */
Std_ReturnType SWCRadar_GetObjects(
    RadarObjectListType *objectList);

/**
 * @brief Check whether radar output is valid.
 *
 * @return true if valid.
 */
bool SWCRadar_IsValid(void);

/**
 * @brief Reset Radar SWC.
 */
void SWCRadar_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SWC_RADAR_H */