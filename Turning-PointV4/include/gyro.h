#pragma once
#include "main.h"

class GyroWrapper
{
    int m_base = 0;
public:
    int Get() { return gyroGet(g_gyro) - m_base; }
    void Reset() { m_base = gyroGet(g_gyro); }
};
