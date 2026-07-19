// display.c
#include <stdio.h>
#include "display.h"

void ShowDisplay(int state)
{
    switch(state)
    {
        case 0:
            printf("Display : NORMAL\n");
            break;

        case 1:
            printf("Display : WARNING\n");
            break;

        case 2:
            printf("Display : BRAKE\n");
            break;

        default:
            printf("Display : UNKNOWN\n");
            break;
    }
}