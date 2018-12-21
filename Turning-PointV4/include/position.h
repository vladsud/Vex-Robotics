#pragma once
#include "Gyro.h"


static const int PositionTrackingRefreshRate = 2;

// This is used to do initial smothening of readings by doing averaging across that many neihbors on each side
// 2 is too small.
// 3-5 seems about right.
static const int initialSmoothingRange = 2;

// This is how many samples to use in both directions (past & future) to calculate velocity of changes.
// The bigger the number - the more fluid calculations we do.
// But, we assume constant acceleration, and that's simply not the case if sample size is big.
// So we should keep it relatively small (which also helps with calculation complexity).
// Also Gyro likely does not need that match smoothing - it intergrates every 2ms, and produces rather good results.
// At the same time, it should be at least 2 * initialSmoothingRange, otherwise we are smoothing twice same data point, likely at expense of accuracy.
// And we have huge latency in motor power (15ms), so acceleration unlikely to change that rapidly.
// 6 or 8 looks good numbers, 10 results in changes being observed  too early / late compared to actual phisics  
static const int finalSmoothingRange = 4;

// Add two extra to reduce chances of running into prboblems. 
static const int SamplesToTrack = 2 + max (2*finalSmoothingRange + initialSmoothingRange + 1, 2*initialSmoothingRange+1);

struct SensorData
{
    double leftEncoder[SamplesToTrack];
    double rightEncoder[SamplesToTrack];
    double sideEncoder[SamplesToTrack];
    double gyro[SamplesToTrack];

    double leftMotorPower[SamplesToTrack];
    double rightMotorPower[SamplesToTrack];

    double shooterAngle[SamplesToTrack];
    double shooterAnglePower[SamplesToTrack];
};

struct PositionData
{
    double leftSpeed[SamplesToTrack];
    double rightSpeed[SamplesToTrack];
    double sideSpeed[SamplesToTrack];
    int gyroSpeed[SamplesToTrack];

    double X[SamplesToTrack];
    double Y[SamplesToTrack];

    int shooterAngle[SamplesToTrack];
};


class PositionTracker
{
    static constexpr double Pi = 3.14159265358;
    static constexpr double GyroToRadiants = Pi / GyroWrapper::Multiplier / 180;
    static constexpr double inchesPerClick = 4.11161263 * Pi / 360; // 0.03588
    static constexpr double distanceRightWheelFromCenter = 7.2908 / inchesPerClick;
    static constexpr double distanceLeftWheelFromCenter = 7.2908 / inchesPerClick;
    static constexpr double one_by_wheelDistance = 1 / (distanceRightWheelFromCenter + distanceLeftWheelFromCenter);

    int m_gyro = GyroWrapper::Get();

    int m_count = 0;

    uint64_t m_time[SamplesToTrack];
    SensorData m_sensor;
    PositionData m_position;
    int m_currentIndex = 0;

public:
    PositionTracker();
    void Update();

private:
    void SmoothSensor();
    bool RecalcPosition(int index);

    template <typename T1>
    T1 SmoothSeries(T1* dataIn);

    template <typename T1, typename T2>
    void SmoothSeries(T1* dataIn, T2* dataSpeedOut);

    template <typename T>
    T& Value(T* data, int index)
    {
        return data[(index + SamplesToTrack) % SamplesToTrack];
    }
};

