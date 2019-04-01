#pragma once
#include "main.h"
#include "pros/adi.hpp"


class GyroWrapper
{
    pros::ADIAnalogIn m_sensor;
    int32_t m_value = 0;
    int32_t m_multiplier = 0;
    uint32_t m_calibValue = 0;
    uint32_t m_stddev = 0;
    int32_t m_limit = 0;
    unsigned long m_lastTime = 0;
    bool m_freeze = false;

  public:
    // Devide by this nuber to convert gyro value to degrees
    static constexpr int Multiplier = 1 << 10;
    int Get() const { return m_value; }
    void SetAngle(int angle) { m_value = angle; }
    void ResetState();

    void Integrate();
    GyroWrapper(unsigned char port, unsigned short multiplier = 0);

    void Freeze() { m_freeze = true; }
    void Unfreeze() { m_freeze = false; }
    void Flip()
    {
        m_value = -m_value;
        m_multiplier = -m_multiplier;
    }
};

class GyroFreezer
{
  GyroWrapper& m_gyro;

public:
  GyroFreezer(GyroWrapper& gyro)
    :m_gyro(gyro)
  {
    m_gyro.Freeze();
  }

  ~GyroFreezer()
  {
    m_gyro.Unfreeze();
  }

};

GyroWrapper &GetGyro();
