/******************************************************************************
 * @file    scheduler.h
 * @brief   ADAS Application Scheduler Interface
 *
 * Defines the initialization and cyclic execution interfaces for the ADAS
 * application scheduler. The scheduler coordinates the execution order of all
 * software components in the system.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/******************************************************************************
 * Scheduler APIs
 ******************************************************************************/

/**
 * @brief Initialize the complete ADAS software stack.
 */
void Scheduler_Init(void);

/**
 * @brief Execute one scheduler cycle.
 *
 * Calls every ADAS software component in the required execution order.
 */
void Scheduler_Run(void);

/**
 * @brief Shutdown the scheduler.
 */
void Scheduler_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULER_H */