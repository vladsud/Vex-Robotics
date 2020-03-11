#include "test.h"
#include "../src/positionCore.cpp"

double PI = PositionTrackerBase::PI;

void ZeroSensor(SensorsRaw& sensor)
{
    sensor.leftEncoder = 0;
    sensor.rightEncoder = 0;
    sensor.leftWheels = 0;
    sensor.rightWheels = 0;
    sensor.sideEncoder = 0;
};

class PositionTest : public PositionTrackerBase
{
protected:
    void ReadSensors(SensorsRaw& sensor) override
    {
        ZeroSensor(sensor);
        sensor.leftEncoder = m_posL;
        sensor.rightEncoder = m_posR;
    }

public:
    double Accelerate(double startingSpeed, double acceleration, unsigned int steps)
    {
        while (steps > 0)
        {
            startingSpeed += acceleration;
            m_posL += startingSpeed;
            m_posR += startingSpeed;

            // printf("A: %f %f\n", m_posL, m_posR);

            Update();

            pros::c::delay(1);
            steps--;
        }
        return startingSpeed;
    }

    double Rotate(double R, double startingAngularVelocity, double angularAcceleration, unsigned int steps)
    {
        while (steps > 0)
        {
            startingAngularVelocity += angularAcceleration;
            m_posL += (R + DISTANCE_LR / 2) / TICKS_TO_IN_LR * startingAngularVelocity;
            m_posR += (R - DISTANCE_LR / 2) / TICKS_TO_IN_LR * startingAngularVelocity;
            m_angle += startingAngularVelocity;

            // printf("%f %f\n", R * startingAngularVelocity, m_angle);

            Update();

            pros::c::delay(1);
            steps--;
        }
        return startingAngularVelocity;
    }

    double PosL() const { return m_posL; }
    double PosR() const { return m_posR; }
    double Angle() const { return m_angle; }

    Position GetPos()
    {
        auto pos = LatestPosition();
        pos.X /= PositionTest::TICKS_TO_IN_LR;
        pos.Y /= PositionTest::TICKS_TO_IN_LR;
        return pos;
    }

protected:
    double m_posL = 0;
    double m_posR = 0;
    double m_angle = 0;
};

static Test testFast("Motion Fast", [] {
    PositionTest test;
    test.Accelerate(0, 1, 40);
    test.Accelerate(40, 0, 60);
    test.Accelerate(40, -1, 40);
    test.Accelerate(0, 0, 100);

    auto distance = 40 * 40 + 40 * 60;

    Assert(test.PosR() == test.PosL());
    auto pos = test.GetPos();
    Assert(pos.X == 0);
    Assert(abs(pos.Y -test.PosL()) < 0.00001);
    Assert(abs(pos.Y - distance) < 0.00001);

    test.Accelerate(0, 0, 100);
    auto pos2 = test.GetPos();
    Assert(pos.X == pos2.X);
    Assert(pos.Y == pos2.Y);
});

static Test testSlow("Motion Slow backwards", [] {
    PositionTest test;
    test.Accelerate(0, -0.01, 1000);
    test.Accelerate(-10, 0, 10000);
    test.Accelerate(-10, 0.01, 1000);

    float distance = - (0.01 * 1000 * 1000 + 10 * 10000);

    Assert(test.PosR() == test.PosL());
    auto pos = test.GetPos();
    Assert(pos.X == 0);
    Assert(abs(pos.Y -test.PosL()) < 1);
    Assert(abs(pos.Y - distance) < 1);
});

void testDiagonal(float angle, int sign = 1)
{
    PositionTest test;

    test.SetAngle(angle);
    test.Accelerate(0, sign * 0.01, 1000);
    test.Accelerate(sign * 10, 0, 10000);
    test.Accelerate(sign * 10, -sign * 0.01, 1000);

    float distance = sign * (0.01 * 1000 * 1000 + 10 * 10000);

    auto pos = test.GetPos();
    angle *=  PI / 180;
    Assert(pos.X * sin(angle) * sign < 0);
    Assert(pos.Y * cos(angle) * sign > 0);
    Assert(abs(pos.X + distance * sin(angle)) < 1);
    Assert(abs(pos.Y - distance * cos(angle)) < 1);
}

static Test testDiag("Motion diagonally", [] {
    testDiagonal(30);
    testDiagonal(45);
    testDiagonal(60);

    testDiagonal(-30);
    testDiagonal(90+30);
    testDiagonal(360-30);
});

static Test testDiagBackwards("Motion diagonally backwards", [] {
    testDiagonal(45, -1);
    testDiagonal(-30, -1);
    testDiagonal(90+30, -1);
    testDiagonal(360-30,-1);
});

static Test testRotate2("Motion rotate uniform", [] {
    PositionTest test;
    float radius = 15;
    test.Rotate(radius, PI / 2000, 0, 1000);
    
    auto pos = test.GetPos();
    Assert(abs(pos.angle + 90) < 0.05);
    Assert(abs(pos.X - pos.Y) < 1);
    Assert(abs(pos.X - radius / PositionTest::TICKS_TO_IN_LR) < 0.5);
});

static Test testRotate("Motion rotate 1", [] {
    PositionTest test;
    float radius = 20;
    test.Rotate(radius, 0, PI / 40 / 2000, 40);
    int steps = (PI / 2 - 2 * test.Angle()) / PI * 2000;
    test.Rotate(20, PI / 2000, 0, steps);
    test.Rotate(20, PI / 2000, -PI / 40 / 2000, 40);

    auto pos = test.GetPos();
    // printf("%f %f %f\n", pos.X, pos.Y, pos.angle);
    Assert(abs(pos.angle + 90) < 0.1);
    Assert(abs(pos.X - pos.Y) < 2);
    Assert(abs(pos.X - radius / PositionTest::TICKS_TO_IN_LR) < 2);
});
