/******************************************************************************
 * @file    constants.h
 * @brief   Project-wide constants for the ADAS Dependency Tracing POC
 *
 * This file contains application-specific constants, default calibration
 * values, CAN identifiers, timeout values, system limits, and diagnostic IDs.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Software Information
 ******************************************************************************/

#define PROJECT_NAME                    "ADAS Dependency Tracing POC"

#define SOFTWARE_VENDOR                 "OpenAI Research"

#define SOFTWARE_VERSION_MAJOR          (1U)
#define SOFTWARE_VERSION_MINOR          (0U)
#define SOFTWARE_VERSION_PATCH          (0U)

/******************************************************************************
 * Scheduler Timing (ms)
 ******************************************************************************/

#define MAIN_TASK_PERIOD_MS             (10U)

#define CAMERA_TASK_PERIOD_MS           (20U)

#define RADAR_TASK_PERIOD_MS            (20U)

#define FUSION_TASK_PERIOD_MS           (20U)

#define FEB_TASK_PERIOD_MS              (10U)

#define HMI_TASK_PERIOD_MS              (50U)

#define DISPLAY_TASK_PERIOD_MS          (100U)

/******************************************************************************
 * Vehicle Limits
 ******************************************************************************/

#define VEHICLE_SPEED_MIN_KPH           (0.0F)

#define VEHICLE_SPEED_MAX_KPH           (250.0F)

#define STEERING_ANGLE_MIN_DEG          (-720.0F)

#define STEERING_ANGLE_MAX_DEG          (720.0F)

/******************************************************************************
 * Default Calibration Values
 ******************************************************************************/

#define DEFAULT_WARNING_TTC_SEC         (2.50F)

#define DEFAULT_BRAKE_TTC_SEC           (1.20F)

#define DEFAULT_COLLISION_PROBABILITY   (0.80F)

#define DEFAULT_MIN_SPEED_KPH           (15.0F)

#define DEFAULT_MAX_OBJECT_DISTANCE_M   (120.0F)

#define DEFAULT_MIN_CONFIDENCE          (0.60F)

/******************************************************************************
 * Camera Parameters
 ******************************************************************************/

#define CAMERA_MAX_RANGE_M              (120.0F)

#define CAMERA_MAX_OBJECTS              (32U)

#define CAMERA_FRAME_RATE_HZ            (30U)

/******************************************************************************
 * Radar Parameters
 ******************************************************************************/

#define RADAR_MAX_RANGE_M               (180.0F)

#define RADAR_MAX_OBJECTS               (32U)

#define RADAR_FRAME_RATE_HZ             (20U)

/******************************************************************************
 * Fusion Parameters
 ******************************************************************************/

#define FUSION_MAX_OBJECTS              (32U)

#define FUSION_MATCH_DISTANCE_M         (2.0F)

#define FUSION_MAX_TRACK_AGE            (5U)

/******************************************************************************
 * Forward Emergency Braking
 ******************************************************************************/

#define FEB_WARNING_LEVEL_1             (1U)

#define FEB_WARNING_LEVEL_2             (2U)

#define FEB_WARNING_LEVEL_3             (3U)

#define FEB_MAX_DECELERATION            (8.5F)

#define FEB_MIN_OPERATIONAL_SPEED       (10.0F)

#define FEB_MAX_OPERATIONAL_SPEED       (180.0F)

/******************************************************************************
 * Signal Timeout (ms)
 ******************************************************************************/

#define SIGNAL_TIMEOUT_CAMERA           (200U)

#define SIGNAL_TIMEOUT_RADAR            (200U)

#define SIGNAL_TIMEOUT_VEHICLE          (100U)

#define SIGNAL_TIMEOUT_FUSION           (150U)

#define SIGNAL_TIMEOUT_HMI              (500U)

/******************************************************************************
 * CAN Message Identifiers
 ******************************************************************************/

#define CAN_ID_VEHICLE_SPEED            (0x100U)

#define CAN_ID_STEERING_ANGLE           (0x101U)

#define CAN_ID_BRAKE_STATUS             (0x102U)

#define CAN_ID_GEAR_POSITION            (0x103U)

#define CAN_ID_CAMERA_OBJECTS           (0x200U)

#define CAN_ID_RADAR_OBJECTS            (0x300U)

#define CAN_ID_FUSION_OUTPUT            (0x400U)

#define CAN_ID_FEB_OUTPUT               (0x500U)

/******************************************************************************
 * Diagnostic Trouble Codes (Example)
 ******************************************************************************/

#define DTC_CAMERA_FAILURE              (0x1001U)

#define DTC_RADAR_FAILURE               (0x1002U)

#define DTC_FUSION_FAILURE              (0x1003U)

#define DTC_FEB_FAILURE                 (0x1004U)

#define DTC_CAN_TIMEOUT                 (0x1005U)

#define DTC_CONFIGURATION_ERROR         (0x1006U)

/******************************************************************************
 * Region Configuration
 ******************************************************************************/

#define REGION_SPEED_LIMIT_EU           (130.0F)

#define REGION_SPEED_LIMIT_US           (120.0F)

#define REGION_SPEED_LIMIT_APAC         (100.0F)

/******************************************************************************
 * Vehicle Variant IDs
 ******************************************************************************/

#define VEHICLE_VARIANT_SEDAN           (1U)

#define VEHICLE_VARIANT_SUV             (2U)

#define VEHICLE_VARIANT_TRUCK           (3U)

/******************************************************************************
 * Feature Flags
 ******************************************************************************/

#define FEATURE_CAMERA_ENABLED          (1U)

#define FEATURE_RADAR_ENABLED           (1U)

#define FEATURE_FUSION_ENABLED          (1U)

#define FEATURE_FEB_ENABLED             (1U)

#define FEATURE_HMI_ENABLED             (1U)

/******************************************************************************
 * Display Configuration
 ******************************************************************************/

#define DISPLAY_REFRESH_PERIOD_MS       (100U)

#define DISPLAY_WARNING_DURATION_MS     (3000U)

/******************************************************************************
 * Logging
 ******************************************************************************/

#define MAX_LOG_ENTRIES                 (1000U)

#define MAX_DIAGNOSTIC_RECORDS          (256U)

/******************************************************************************
 * Object Validation
 ******************************************************************************/

#define MIN_OBJECT_WIDTH_M              (0.30F)

#define MAX_OBJECT_WIDTH_M              (5.00F)

#define MIN_OBJECT_HEIGHT_M             (0.30F)

#define MAX_OBJECT_HEIGHT_M             (4.50F)

/******************************************************************************
 * Floating Point Precision
 ******************************************************************************/

#define FLOAT_EPSILON                   (0.0001F)

#ifdef __cplusplus
}
#endif

#endif /* CONSTANTS_H */