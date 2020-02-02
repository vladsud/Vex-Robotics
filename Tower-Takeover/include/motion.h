#pragma once

#include "main.h"

int Sign(int value);

unsigned int SpeedFromDistances(unsigned int distance, const unsigned int* points, const unsigned int* speeds);
int SpeedFromDistances(int distance, const unsigned int* points, const unsigned int* speeds);

struct Model
{
    // Power to use when starting motion.Used to improve precision -
    // putting all power to wheels momentaraly results in wheel sleepage / robot sliding 
    unsigned int initialPower;
    // amount of power to add on each cycle (10ms).
    // 1 = full power in 1.3 seconds
    // (45, 2) - full power in 410ms
    unsigned int initialPowerDelta;

    // These 3 cofficients are used to calculate power
    int basePower;
    float speedCoeff;
    float diffCoeff;

    unsigned int points[8];
    unsigned int speeds[8];
};

extern const Model moveModel;
extern const Model turnModel;

struct Motion
{
    Motion(const Model& model, unsigned int speedLimit = UINT_MAX);

    // error, or distance to the target
    virtual int GetError() = 0;

    // sets motor power - positive for positive error
    virtual void SetMotorPower(int power) = 0;

    // Actual logic
    bool ShouldStop();

    int GetStopPoint() { return m_stopPoint; }

    // Returns ideal speed depending on distance to target (error)
    // Positive error assumes positive speed on the way to target and generates positive power.
    int SpeedFromDistance(int error);

private:
    const Model m_model;
    const unsigned int m_speedLimit;
    int m_power = 0;
    int m_lastError = 0;
    int m_initialError;
    int m_stopPoint = UINT_MAX;
    bool m_firstRun = true;
};

