#pragma once

struct SensorsRaw {
    int leftEncoder;
    int rightEncoder;
    int rightWheels;
    int leftWheels;
    int sideEncoder;
    float angle = 0; // not used
};

struct Sensors {
    // Given that we are doing fusion in SynthesizeSensors(), these can't be ints!
    double leftEncoder;
    double rightEncoder;
    double rightWheels;
    double leftWheels;
    double sideEncoder;
    float angle;
};

template <typename T = Sensors>
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
    T last;
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
    static constexpr double DISTANCE_S = 0;  // inches

    static constexpr float TICKS_TO_IN_LR = WHEEL_DIAMETER_IN_LR * PI / TICKS_PER_ROTATION;
    static constexpr float TICKS_TO_IN_S = WHEEL_DIAMETER_IN_S * PI / TICKS_PER_ROTATION;

protected:
    // Synthesized sensor data
    Sensors m_sensors {};
    SensorSpeed<Sensors> m_sensorDelta;

    // Speed over 10ms
    SensorSpeed<SensorsRaw> m_sensorSpeedSlow;

    // Clculated position
    Position m_position;

    double m_angleOffset = 0;

protected:
    virtual void ReadSensors(SensorsRaw& sensor) = 0;

public:
    PositionTrackerBase();
    void ResetState();

    void Update();

    Position LatestPosition();
    void SetCoordinates(Position coord);
    double GetAngle();
    void SetAngle(float angle);

    float GetRobotVelocity();
    int GetLeftPos();
    int GetRightPos();
};
