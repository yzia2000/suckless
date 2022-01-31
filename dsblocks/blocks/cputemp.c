#include <stdio.h>

#include "../util.h"
#include "cputemp.h"

#define ICON0                           COL1 "" COL0
#define ICON1                           COL2 "" COL0

#define WARNCPUTEMP                     70000

#define CPUTEMPFILE                     "/sys/class/thermal/thermal_zone0/temp"

void
cputempu(char *str, int sigval)
{
        int temp;

        if (!readint(CPUTEMPFILE, &temp)) {
                *str = '\0';
                return;
        }
        if (temp < WARNCPUTEMP)
                snprintf(str, BLOCKLENGTH, ICON0 "%d°C", temp / 1000);
        else
                snprintf(str, BLOCKLENGTH, ICON1 "%d°C", temp / 1000);
}
