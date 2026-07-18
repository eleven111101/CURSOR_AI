/******************************************************************************
 * @file feature_config.h
 ******************************************************************************/

#ifndef FEATURE_CONFIG_H
#define FEATURE_CONFIG_H

#include "types.h"

typedef struct
{
    bool cameraEnabled;

    bool radarEnabled;

    bool fusionEnabled;

    bool febEnabled;

    bool hmiEnabled;

} FeatureConfigType;

const FeatureConfigType*
FeatureConfig_Get(void);

#endif