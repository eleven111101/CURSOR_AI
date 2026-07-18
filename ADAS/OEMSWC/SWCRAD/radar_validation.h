/******************************************************************************
 * @file    radar_validation.h
 * @brief   Radar Validation Interface
 *
 * Performs validation and filtering of raw radar detections before they are
 * consumed by the Radar SWC.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef RADAR_VALIDATION_H
#define RADAR_VALIDATION_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "swc_radar.h"

/******************************************************************************
 * Macros
 ******************************************************************************/

#define RADAR_MIN_RANGE           (0.5F)
#define RADAR_MAX_RANGE           (200.0F)
#define RADAR_MAX_REL_SPEED       (120.0F)

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize radar validation.
 */
void RadarValidation_Init(void);

/**
 * @brief Execute one radar validation cycle.
 */
void RadarValidation_MainFunction(void);

/**
 * @brief Validate radar detections.
 *
 * @return E_OK if validation succeeds.
 */
Std_ReturnType RadarValidation_Process(void);

/**
 * @brief Retrieve validated radar objects.
 *
 * @param objectList Pointer to destination.
 *
 * @return E_OK if successful.
 */
Std_ReturnType RadarValidation_GetObjects(
    RadarObjectListType *objectList);

/**
 * @brief Reset radar validation.
 */
void RadarValidation_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* RADAR_VALIDATION_H */