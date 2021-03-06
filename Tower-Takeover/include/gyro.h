#pragma once

#include "pros/adi.hpp"

class LegacyGyro
{
    pros::ADIAnalogIn m_sensor;
    int64_t m_value = 0;
    uint32_t m_calibValue = 0;
    unsigned long m_lastTime = 0;
    const float m_multiplier;
    const int32_t m_limit;

  public:
    float GetAngle() const;
    void SetAngle(float angle);
    void ResetState();

    void Integrate();
    LegacyGyro(unsigned char port, unsigned short multiplier = 0);
};

class GyroNothing
{
  public:
    float GetAngle() const;
    void SetAngle(float angle) {}
    void ResetState() {}
    void Integrate() {}
    GyroNothing() {}
    GyroNothing(unsigned int) {}
};

class GyroInertial
{
    const uint8_t m_port;
    float m_offset = 0;

  public:
    float GetAngle() const;
    void SetAngle(float angle);
    void ResetState();

    void Integrate() {}
    GyroInertial(uint8_t port);
};

class GyroBoth
{
    GyroNothing m_gyroImu {gyroPortImu}; // GyroInertial
    GyroNothing m_gyro {gyroPort}; // LegacyGyro
    GyroNothing m_gyro2 {gyroPort2}; // LegacyGyro
    unsigned int m_count = 0;

  public:
    GyroBoth();

    void Integrate();

    float GetAngle() const;
    void SetAngle(float angle);
    void ResetState();
};

using GyroWrapper = GyroBoth;
GyroWrapper &GetGyro();
