/******************************************************************************
 * @file vehicle_variant.h
 ******************************************************************************/

#ifndef VEHICLE_VARIANT_H
#define VEHICLE_VARIANT_H

#include "types.h"

typedef enum
{
    VARIANT_SEDAN = 0,
    VARIANT_SUV,
    VARIANT_TRUCK

} VehicleVariantType;

typedef struct
{
    VehicleVariantType variant;

    RegionType region;

    float32 maximumSpeed;

} VehicleVariantConfigType;

const VehicleVariantConfigType*
VehicleVariant_Get(void);

#endif