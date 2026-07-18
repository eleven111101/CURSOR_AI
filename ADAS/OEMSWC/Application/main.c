/******************************************************************************
 * @file    main.c
 * @brief   ADAS Application Entry Point
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "scheduler.h"

int main(void)
{
    Scheduler_Init();

    while (1)
    {
        Scheduler_Run();
    }

    /* Never reached */
    Scheduler_Shutdown();

    return 0;
}