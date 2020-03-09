#include "test.h"
#include "../src/positionCore.cpp"

void ZeroSensor(Sensors& sensor)
{
    sensor.angle = 0;
    sensor.leftEncoder = 0;
    sensor.rightEncoder = 0;
    sensor.leftWheels = 0;
    sensor.rightWheels = 0;
    sensor.sideEncoder = 0;
};

class PositionTest : public PositionTrackerBase
{
public:
    PositionTest(
        float acceleration,
        float accelerationSteps,
        float constantSteps)
        : m_acceleration(acceleration),
        m_accelerationSteps(accelerationSteps),
        m_constantSteps(constantSteps)
    {
    }

    float Pos() const { return m_pos; }

protected:
    void ReadSensors(Sensors& sensor) override
    {
        ZeroSensor(sensor);

        m_steps++;
        if (m_steps <= m_accelerationSteps)
            m_pos += m_steps * m_acceleration;
        else if (m_steps <= m_accelerationSteps + m_constantSteps)
            m_pos += m_accelerationSteps * m_acceleration;
        else if (m_steps <= 2 * m_accelerationSteps + m_constantSteps)
            m_pos += (2 * m_accelerationSteps + m_constantSteps - m_steps) * m_acceleration;

        sensor.leftEncoder = m_pos;
        sensor.rightEncoder = m_pos;
    }
public:    
    const float m_acceleration;
    const float m_accelerationSteps;
    const float m_constantSteps;

protected:
    float m_pos = 0;
    float m_speed = 0;
    float m_steps = 0;
};

static void RunModel()
{
    ResetTime();

    PositionTest test(0.1, 50, 50);
    int steps = 200;

    while (steps > 0) {
        steps--;
        test.Update();
        pros::c::delay(1);
    }

    auto pos = test.LatestPosition();
    pos.X /= PositionTest::TICKS_TO_IN_LR;
    pos.Y /= PositionTest::TICKS_TO_IN_LR;

    Assert(pos.Y == 0);
    Assert(abs(pos.X - test.Pos()) < 2);
}


static Test test("Motion", [] {
    RunModel();
});
