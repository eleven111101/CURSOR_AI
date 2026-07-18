/******************************************************************************
 * @file feature_config.c
 ******************************************************************************/

#include "feature_config.h"

static FeatureConfigType gFeatureConfig =
{
    .cameraEnabled = true,
    .radarEnabled = true,
    .fusionEnabled = true,
    .febEnabled = true,
    .hmiEnabled = true
};

const FeatureConfigType*
FeatureConfig_Get(void)
{
    return &gFeatureConfig;
}