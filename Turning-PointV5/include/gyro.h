#pragma once
#include "main.h"

class GyroWrapper
{
    int32_t m_value = 0;
    int32_t m_multiplier;
    uint32_t m_calibValue = 0;
    uint32_t m_stddev = 0;
    int32_t m_limit = 0;
    unsigned long m_lastTime = 0;
    unsigned char m_port;

  public:
    // Devide by this nuber to convert gyro value to degrees
    static constexpr int Multiplier = 1 << 10;
    int Get() const { return m_value; }
    void SetAngle(int angle) { m_value = angle; }
    void ResetState() { m_lastTime = pros::c::millis() - 1; }

    void Integrate();
    GyroWrapper(unsigned char port, unsigned short multiplier = 0);

    void Flip()
    {
        m_value = -m_value;
        m_multiplier = -m_multiplier;
    }
};

GyroWrapper &GetGyro();
