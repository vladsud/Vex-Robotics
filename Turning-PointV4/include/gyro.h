#pragma once
#include "main.h"

class GyroWrapper
{
public:
    static const int Multiplier = 256;
    
    // Positive is counterclockwise
    static int Get()
    {
        // Based on Pros implementaitno - this is 256x of what gyroGet returns!!! 
        int result = *(int32_t*)g_gyro + 128;
        int gyroRes = gyroGet(g_gyro);
        if (abs(result - gyroRes * Multiplier) >= Multiplier)
        {
            printf("Gyro does not work: %d, %d\n", result, gyroRes);
            Assert(false);
        }
        return result;
        // return gyroGet(g_gyro);
    }
};
