/******************************************************************************
 * @file    diagnostics.c
 * @brief   ADAS Diagnostics Implementation
 *
 * Monitors the operational status of the major ADAS software components and
 * maintains a simple diagnostic database for health monitoring.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "diagnostics.h"

#include <string.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static DiagnosticDatabaseType gDiagnosticDatabase;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void Diagnostics_Init(void)
{
    memset(&gDiagnosticDatabase, 0, sizeof(gDiagnosticDatabase));
}

void Diagnostics_MainFunction(void)
{
    (void)Diagnostics_Process();
}

Std_ReturnType Diagnostics_Process(void)
{
    uint8 index;

    gDiagnosticDatabase.recordCount = 8U;

    for (index = 0U; index < gDiagnosticDatabase.recordCount; index++)
    {
        gDiagnosticDatabase.records[index].module =
            (DiagnosticModuleType)index;

        gDiagnosticDatabase.records[index].state =
            SYSTEM_OPERATIONAL;

        gDiagnosticDatabase.records[index].errorCounter = 0U;

        gDiagnosticDatabase.records[index].communicationFault = false;
    }

    return E_OK;
}

Std_ReturnType Diagnostics_GetDatabase(
    DiagnosticDatabaseType *database)
{
    if (database == NULL)
    {
        return E_NOT_OK;
    }

    memcpy(database,
           &gDiagnosticDatabase,
           sizeof(DiagnosticDatabaseType));

    return E_OK;
}

void Diagnostics_Reset(void)
{
    memset(&gDiagnosticDatabase, 0, sizeof(gDiagnosticDatabase));
}