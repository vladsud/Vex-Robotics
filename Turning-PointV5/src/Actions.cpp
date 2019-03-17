#include "actions.h"

// unsigned int points[] {30, 50, 1600, UINT_MAX};
// unsigned int speeds[] {0,  60, 4000, 4000};
unsigned int SpeedFromDistances(unsigned int distance, const unsigned int* points, const unsigned int* speeds)
{
    unsigned int lowPoint = 0;
    unsigned int lowSpeed = 0;
    while (true)
    {
        unsigned int highPoint = *points;
        unsigned int highSpeed = *speeds;
        Assert(lowPoint < highPoint);
        Assert(lowSpeed <= highSpeed);
        if (distance <= highPoint)
            return lowSpeed + (highSpeed - lowSpeed) * (distance - lowPoint) / (highPoint - lowPoint);
        points++;
        speeds++;
        lowPoint = highPoint;
        lowSpeed = highSpeed;
    }
}

int SpeedFromDistances(int distance, const unsigned int* points, const unsigned int* speeds)
{
    return SpeedFromDistances((unsigned int)abs(distance), points, speeds) * Sign(distance);
}
