/******************************************************************************
 * @file vehicle_variant.c
 ******************************************************************************/

#include "vehicle_variant.h"

static VehicleVariantConfigType gVariant =
{
    .variant = VARIANT_SEDAN,
    .region = REGION_EU,
    .maximumSpeed = 180.0F
};

const VehicleVariantConfigType*
VehicleVariant_Get(void)
{
    return &gVariant;
}