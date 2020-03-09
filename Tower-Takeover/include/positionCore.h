#pragma once

struct Sensors {
    int leftEncoder;
    int rightEncoder;
    int rightWheels;
    int leftWheels;
    int sideEncoder;
    float angle;
};

struct SensorSpeed
{
    // Note we are not that sensetive to precision here - switching to floats
    // will give us below 1mm error on 1 min streight run.
    // Not sure about turning though
    double leftEncoder;
    double rightEncoder;
    double rightWheels;
    double leftWheels;
    double sideEncoder;
    double angle;
    Sensors last;
    unsigned int time;
};

// This is the structure we expose to other modules
struct Position
{
    double X;
    double Y;
    double angle;
};

struct PositionSpeed
{
    Position pos;
    Position last;
    unsigned int time;
};

// Main class
class PositionTrackerBase
{
public:
    static constexpr float PI = 3.14159265358;
    static constexpr int TICKS_PER_ROTATION = 360;
    static constexpr float AngleToRadiants = PI / 180;

    static constexpr float WHEEL_DIAMETER_IN_LR = 2.783;
    static constexpr float WHEEL_DIAMETER_IN_S = 2.783;
    static constexpr float DISTANCE_LR = 10.0; // inches
    static constexpr double DISTANCE_S = 5.0;  // inches

    static constexpr float TICKS_TO_IN_LR = WHEEL_DIAMETER_IN_LR * PI / TICKS_PER_ROTATION;
    static constexpr float TICKS_TO_IN_S = WHEEL_DIAMETER_IN_S * PI / TICKS_PER_ROTATION;

protected:
    // Synthesized sensor data
    Sensors m_sensors {};
    SensorSpeed m_sensorSpeed;

    // Speed over 10ms
    SensorSpeed m_sensorSpeedSlow;

    // Clculated position
    Position m_position;

protected:
    virtual void ReadSensors(Sensors& sensor) = 0;

    static void UpdateSensorSpeed(const Sensors& pos, SensorSpeed& speed, unsigned int timeDiff);
    static void InitSensorSpeed(const Sensors& pos, SensorSpeed& speed);
    static void SynthesizeSensors(const Sensors& pos, const SensorSpeed& speed, Sensors& posOut);

public:
    PositionTrackerBase();

    void Update();

    Position LatestPosition();
    void SetCoordinates(Position coord);

    float GetRobotVelocity();
    int GetLeftPos();
    int GetRightPos();
};
