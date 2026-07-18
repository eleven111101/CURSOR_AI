/******************************************************************************
 * @file    config.h
 * @brief   Global ADAS Configuration Interface
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/******************************************************************************
 * Global Configuration
 ******************************************************************************/

typedef struct
{
    uint16 softwareVersionMajor;
    uint16 softwareVersionMinor;
    uint16 softwareVersionPatch;

    bool diagnosticsEnabled;
    bool loggingEnabled;
    bool simulationMode;

} SystemConfigType;

/******************************************************************************
 * APIs
 ******************************************************************************/

void Config_Init(void);

const SystemConfigType* Config_GetSystemConfig(void);

#ifdef __cplusplus
}
#endif

#endif