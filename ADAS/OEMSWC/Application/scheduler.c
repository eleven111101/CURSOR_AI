/******************************************************************************
 * @file    scheduler.c
 * @brief   ADAS Application Scheduler Implementation
 *
 * Initializes and executes the ADAS software stack in the required order.
 *
 * Project : ADAS Dependency Tracing POC
 ******************************************************************************/

#include "scheduler.h"

#include "can_if.h"
#include "vehstatus_in.h"
#include "rte.h"

#include "camera_if.h"
#include "camera_processing.h"
#include "lane_detection.h"
#include "object_detection.h"
#include "camera_output.h"

#include "radar_validation.h"
#include "swc_radar.h"

#include "fusion_filter.h"
#include "swc_fusion.h"

#include "feb_logic.h"
#include "feb_output.h"
#include "swc_feb.h"

#include "swc_hmi.h"
#include "display_manager.h"
#include "diagnostics.h"

#include <stdbool.h>

/******************************************************************************
 * Static Data
 ******************************************************************************/

static bool gSchedulerInitialized = false;

/******************************************************************************
 * Global Functions
 ******************************************************************************/

void Scheduler_Init(void)
{
    CANIF_Init();
    VehStatus_Init();

    RTE_Init();

    CameraIF_Init();
    CameraProcessing_Init();
    LaneDetection_Init();
    ObjectDetection_Init();
    CameraOutput_Init();

    RadarValidation_Init();
    SWCRadar_Init();

    FusionFilter_Init();
    SWCFusion_Init();

    FEBLogic_Init();
    FEBOutput_Init();
    SWCFEB_Init();

    SWCHMI_Init();

    DisplayManager_Init();

    Diagnostics_Init();

    gSchedulerInitialized = true;
}

void Scheduler_Run(void)
{
    if (!gSchedulerInitialized)
    {
        return;
    }

    /* Communication */
    CANIF_MainFunction();
    VehStatus_MainFunction();

    /* Camera Pipeline */
    CameraIF_MainFunction();
    CameraProcessing_MainFunction();
    LaneDetection_MainFunction();
    ObjectDetection_MainFunction();
    CameraOutput_MainFunction();

    /* Radar Pipeline */
    RadarValidation_MainFunction();
    SWCRadar_MainFunction();

    /* Sensor Fusion */
    FusionFilter_MainFunction();
    SWCFusion_MainFunction();

    /* Decision Making */
    FEBLogic_MainFunction();
    SWCFEB_MainFunction();

    /* Driver Notification */
    SWCHMI_MainFunction();
    DisplayManager_MainFunction();

    /* Diagnostics */
    Diagnostics_MainFunction();

    /* Runtime Environment */
    RTE_MainFunction();
}

void Scheduler_Shutdown(void)
{
    gSchedulerInitialized = false;
}