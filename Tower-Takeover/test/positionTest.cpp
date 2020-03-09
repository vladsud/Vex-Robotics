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

static Position RunModel(PositionTest& model, unsigned int steps)
{
    ResetTime();
    while (steps > 0) {
        steps--;
        model.Update();
        pros::c::delay(1);
    }

    auto pos = model.LatestPosition();
    pos.X /= PositionTest::TICKS_TO_IN_LR;
    pos.Y /= PositionTest::TICKS_TO_IN_LR;
    return pos;
}

static Test test("Motion", [] {
    PositionTest test1(1, 40, 60000);
    auto pos = RunModel(test1, 70000);
    Assert(pos.X == 0);
    Assert(pos.Y == test1.Pos());

    PositionTest test2(0.01, 40, 10000);
    pos = RunModel(test2, 10100);
    Assert(pos.X == 0);
    auto diff = pos.Y - test2.Pos();
    Assert(diff <= 0);
    // due to sensors using integers, and rounding errors in SynthesizeSensors(), we can get up to 2mm of error accumulation.
    Assert(diff >= -2);
});
