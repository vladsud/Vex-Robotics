#pragma once

#include "Gyro.h"

float GetGyroReading();

struct Sensors {
    int leftEncoder;
    int rightEncoder;
    int rightWheels;
    int leftWheels;
    int sideEncoder;
    float angle;
};

struct SensorSpeed : public Sensors
{
    Sensors last;
    unsigned int time;
};

// This is the structure we expose to other modules
struct Position
{
    double X;
    double Y;
    float angle;
};

struct PositionSpeed
{
    Position pos;
    Position last;
    unsigned int time;
};


/*******************************************************************************
 *
 * Encoder class
 * 
 ******************************************************************************/
class Encoder
{
public:
  Encoder(std::uint8_t portTop, std::uint8_t portBottom, bool reverse);
  // Resets encoder for all usages:
  // All external accesses to this encoder will see current pos being reset
  void HardReset();
  // Returns cached position
  int GetPos();

private:
  pros::c::adi_encoder_t m_encoder;
};


/*******************************************************************************
 *
 * CMotor class
 * 
 ******************************************************************************/
class CMotor
{
public:
  CMotor(unsigned int port);
  // Resets motor for all usages:
  // All external accesses to this motor will see current pos being reset
  void HardReset();
  // Returns cached position
  int GetPos();

private:
  const unsigned int m_port;
};


// Main class
class PositionTracker
{
public:
    static constexpr float WHEEL_DIAMETER_IN_LR = 2.783;
    static constexpr float WHEEL_DIAMETER_IN_S = 2.783;
    static constexpr float PI = 3.14159265358;
    static constexpr int TICKS_PER_ROTATION = 360;
    static constexpr float SPIN_TO_IN_LR = WHEEL_DIAMETER_IN_LR * PI / TICKS_PER_ROTATION; // 0.024
    static constexpr float SPIN_TO_IN_S = WHEEL_DIAMETER_IN_S * PI / TICKS_PER_ROTATION;
    static constexpr float AngleToRadiants = PI / 180;
    // static constexpr float inchesPerClick = 4.11161263 * PI / TICKS_PER_ROTATION; // 0.03588
    static constexpr float DISTANCE_LR = 10.0;
    static constexpr double DISTANCE_S = 5.0;

private:
    CMotor m_motorLeftFront {leftFrontDrivePort};
    CMotor m_motorLeftBack {leftBackDrivePort};
    CMotor m_motorRightFront {rightFrontDrivePort};
    CMotor m_motorRightBack {rightBackDrivePort};
    Encoder m_leftEncoder {leftEncoderPortTop, leftEncoderPortBottom, true};
    Encoder m_rightEncoder {rightEncoderPortTop, rightEncoderPortBottom, true};

    int m_count = 0;
    bool m_flipX = false;
    float m_angleOffset = 0;

    // Raw data from sensors
    Sensors m_sensorsRaw;
    SensorSpeed m_sensorSpeedRaw;

    // Synthesized sensor data
    Sensors m_sensors;
    SensorSpeed m_sensorSpeed;

    // Speed over 10ms
    SensorSpeed m_sensorSpeedSlow;

    // Clculated position
    Position m_position;

private:
    void ReadSensors(Sensors& sensor);
    void UpdateSensorSpeed(const Sensors& pos, SensorSpeed& speed, unsigned int timeDiff);
    void InitSensorSpeed(const Sensors& pos, SensorSpeed& speed);
    void SynthesizeSensors(const Sensors& pos, const SensorSpeed& speed, Sensors& posOut);

public:
    PositionTracker();
    // Recalc. Expensive operation that should happen time to time
    void Update();
    void ResetState();

    int GetRobotVelocity();
    Position LatestPosition();
    void SetCoordinates(Position cord);
    void SetAngle(int degrees);
    float GetAngle();
    void FlipX(bool flip);

    int GetLeftPos();
    int GetRightPos();
};

PositionTracker &GetTracker();
