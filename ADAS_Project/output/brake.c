// brake.c
#include <stdio.h>
#include "brake.h"

void ApplyBrake(int status)
{
    if(status)
        printf("Brake Applied\n");
    else
        printf("Brake Released\n");
}