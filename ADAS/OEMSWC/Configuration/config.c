/******************************************************************************
 * @file config.c
 ******************************************************************************/

#include "config.h"

static SystemConfigType gSystemConfig =
{
    .softwareVersionMajor = 1U,
    .softwareVersionMinor = 0U,
    .softwareVersionPatch = 0U,

    .diagnosticsEnabled = true,
    .loggingEnabled = true,
    .simulationMode = false
};

void Config_Init(void)
{
    /* Future EEPROM/NVM initialization */
}

const SystemConfigType* Config_GetSystemConfig(void)
{
    return &gSystemConfig;
}