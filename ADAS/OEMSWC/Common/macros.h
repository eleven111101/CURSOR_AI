/******************************************************************************
 * @file    macros.h
 * @brief   Common macro definitions for the ADAS Dependency Tracing POC
 *
 * This file contains generic utility macros used throughout the OEM software
 * components. Configuration values and calibration parameters are defined in
 * constants.h.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef MACROS_H
#define MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Generic Boolean Macros
 ******************************************************************************/

#ifndef TRUE
#define TRUE                        (1U)
#endif

#ifndef FALSE
#define FALSE                       (0U)
#endif

#ifndef NULL_PTR
#define NULL_PTR                    ((void *)0)
#endif

/******************************************************************************
 * General Utility Macros
 ******************************************************************************/

#define UNUSED(x)                   ((void)(x))

#define ARRAY_SIZE(x)               (sizeof(x) / sizeof((x)[0]))

#define ABS(x)                      (((x) < 0) ? -(x) : (x))

#define MIN(a,b)                    (((a) < (b)) ? (a) : (b))

#define MAX(a,b)                    (((a) > (b)) ? (a) : (b))

#define CLAMP(x,min,max)            (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))

#define SWAP(type,a,b)              \
do                                  \
{                                   \
    type temp = (a);                \
    (a) = (b);                      \
    (b) = temp;                     \
} while(0)

/******************************************************************************
 * Bit Manipulation
 ******************************************************************************/

#define BIT(x)                      (1UL << (x))

#define SET_BIT(value,bit)          ((value) |= BIT(bit))

#define CLEAR_BIT(value,bit)        ((value) &= ~(BIT(bit)))

#define TOGGLE_BIT(value,bit)       ((value) ^= BIT(bit))

#define CHECK_BIT(value,bit)        (((value) & BIT(bit)) != 0U)

/******************************************************************************
 * Mathematical Conversions
 ******************************************************************************/

#define DEG_TO_RAD(x)               ((x) * 0.01745329252F)

#define RAD_TO_DEG(x)               ((x) * 57.29577951F)

#define KPH_TO_MPS(x)               ((x) / 3.6F)

#define MPS_TO_KPH(x)               ((x) * 3.6F)

/******************************************************************************
 * Floating Point Comparison
 ******************************************************************************/

#define FLOAT_EQUAL(a,b,eps)        (ABS((a)-(b)) < (eps))

/******************************************************************************
 * Object Limits
 ******************************************************************************/

#define MAX_CAMERA_OBJECTS          (32U)

#define MAX_RADAR_OBJECTS           (32U)

#define MAX_FUSION_OBJECTS          (32U)

#define MAX_TRACKED_OBJECTS         (64U)

/******************************************************************************
 * Buffer Sizes
 ******************************************************************************/

#define CAN_RX_BUFFER_SIZE          (256U)

#define CAN_TX_BUFFER_SIZE          (256U)

#define DIAG_BUFFER_SIZE            (64U)

#define EVENT_LOG_SIZE              (100U)

/******************************************************************************
 * Alive Counter
 ******************************************************************************/

#define ALIVE_COUNTER_MIN           (0U)

#define ALIVE_COUNTER_MAX           (255U)

#define ALIVE_COUNTER_INCREMENT(x)  \
    (((x) >= ALIVE_COUNTER_MAX) ? ALIVE_COUNTER_MIN : ((x) + 1U))

/******************************************************************************
 * Timeout Handling
 ******************************************************************************/

#define TIMER_EXPIRED(current,start,timeout) \
    (((current) - (start)) >= (timeout))

/******************************************************************************
 * Software Version
 ******************************************************************************/

#define SOFTWARE_MAJOR_VERSION      (1U)

#define SOFTWARE_MINOR_VERSION      (0U)

#define SOFTWARE_PATCH_VERSION      (0U)

/******************************************************************************
 * Feature Enable / Disable
 ******************************************************************************/

#define FEATURE_ENABLED             (1U)

#define FEATURE_DISABLED            (0U)

/******************************************************************************
 * Scheduler
 ******************************************************************************/

#define TASK_ENABLE                 (1U)

#define TASK_DISABLE                (0U)

/******************************************************************************
 * Common Status
 ******************************************************************************/

#define STATUS_VALID                (1U)

#define STATUS_INVALID              (0U)

#define STATUS_ACTIVE               (1U)

#define STATUS_INACTIVE             (0U)

/******************************************************************************
 * Logging
 ******************************************************************************/

#define LOG_LEVEL_ERROR             (0U)

#define LOG_LEVEL_WARNING           (1U)

#define LOG_LEVEL_INFO              (2U)

#define LOG_LEVEL_DEBUG             (3U)

/******************************************************************************
 * Compile-Time Helper
 ******************************************************************************/

#define STATIC                      static

#define INLINE                      inline

#define CONST                       const

/******************************************************************************
 * Memory Initialization
 ******************************************************************************/

#define CLEAR_MEMORY(ptr,size)      memset((ptr), 0, (size))

#define COPY_MEMORY(dst,src,size)   memcpy((dst), (src), (size))

#ifdef __cplusplus
}
#endif

#endif /* MACROS_H */