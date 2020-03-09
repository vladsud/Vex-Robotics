#include "test.h"
#include "../src/Motion.cpp"

struct TestMotion : public Motion
{
    TestMotion(int distance, unsigned int speedLimit = UINT_MAX)
        : Motion(moveModel, speedLimit)
        , m_distance(distance)
    {
    }

    int GetError() override
    {
        return m_distance;
    }

    void SetMotorPower(int power) override
    {
        int speed = m_speed;
        if (abs(m_speed) <= 0.5)
            m_speed = 0;
        else
            m_speed -= Sign(m_speed) / 2;

        // Glancing at charts, there is almost constant acceleration up until
        // we reach within 5-10% of final speed (at which point accelration slows down
        // and is more logarithmic)  
        int topSpeed = power * 70 / 127;
        if (power * m_speed < 0)
        {
            m_speed += power * 20 / 127;
        }
        else if (abs(topSpeed) > abs(m_speed))
        {
            m_speed += power * 1.8 / 127;
        }
        else
        {
            m_speed += power / 127 / 4;
        }

        m_distance -= (m_speed + speed) / 2;
        m_power = power;
    }

    float m_distance;
    float m_speed = 0;
    int m_power = 0;
};

struct MotionResults
{
    unsigned int iterations;
    int stopPoint;
};

static MotionResults RunModel(TestMotion& motion)
{
    // EnableConsoleLogs(Log::Motion);

    ResetTime();
    int distance = motion.m_distance;
    while (true) {
        if (motion.ShouldStop())
            break;
        AssertSz(pros::c::millis() < 200, "Too many iterations");
        pros::c::delay(1);
    }

    AssertSz(abs(motion.GetStopPoint()) < 15, "Too many iterations");
    Test::Report("   %d iterations, stop point = %d\n", pros::c::millis(), motion.GetStopPoint());

    return {pros::c::millis(), motion.GetStopPoint()};
}

static MotionResults RunModels(TestMotion& motion1, TestMotion& motion2)
{
    auto res1 = RunModel(motion1);
    auto res2 = RunModel(motion2);
    Assert(res1.iterations == res2.iterations);
    Assert(res1.stopPoint == -res2.stopPoint);
    return res1;
}

static Test test("Motion", [] {
    auto res1 = RunModels(TestMotion(3000), TestMotion(-3000));
    auto res2 = RunModels(TestMotion(1000), TestMotion(-1000));
    Assert(res2.iterations < res1.iterations);
    auto res3 = RunModels(TestMotion(3000, 50), TestMotion(-3000, 50));
    Assert(res3.iterations > res1.iterations);
});
