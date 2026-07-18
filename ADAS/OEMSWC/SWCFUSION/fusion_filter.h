/******************************************************************************
 * @file    fusion_filter.h
 * @brief   Sensor Fusion Filter Interface
 *
 * Performs association and fusion of Camera and Radar objects to generate a
 * unified object list for the FEB software component.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef FUSION_FILTER_H
#define FUSION_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "camera_output.h"
#include "swc_radar.h"
#include "swc_fusion.h"

/******************************************************************************
 * Fusion Parameters
 ******************************************************************************/

#define FUSION_DISTANCE_THRESHOLD      (2.0F)
#define FUSION_MAX_TTC                (99.9F)
#define FUSION_MIN_RELATIVE_SPEED     (0.1F)

/******************************************************************************
 * APIs
 ******************************************************************************/

/**
 * @brief Initialize fusion filter.
 */
void FusionFilter_Init(void);

/**
 * @brief Execute one fusion filter cycle.
 */
void FusionFilter_MainFunction(void);

/**
 * @brief Fuse Camera and Radar objects.
 *
 * @param cameraOutput Pointer to camera output.
 * @param radarObjects Pointer to validated radar objects.
 * @param fusionObjects Pointer to fused object list.
 *
 * @return E_OK if fusion succeeds.
 */
Std_ReturnType FusionFilter_Process(
    const CameraOutputType *cameraOutput,
    const RadarObjectListType *radarObjects,
    FusionObjectListType *fusionObjects);

/**
 * @brief Reset fusion filter.
 */
void FusionFilter_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* FUSION_FILTER_H */