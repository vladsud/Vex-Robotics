#include "main.h"

int count = 0;

void PrintSensors()
{
    count++;
    if (count % 50 == 0)
    {
        printf("LD: %d     LL: %d     A: %d     S:%d    RL: %d     RD: %d\n",
               encoderGet(leftDriveEncoder), analogRead(LEFTLIFTPOTPORT),
               analogRead(ANGLEPOTPORT), analogRead(SPINNERPOTPORT),
               analogRead(RIGHTLIFTPOTPORT), encoderGet(rightDriveEncoder));
    }
}