#include "test.h"
#include "../src/positionCore.cpp"
#include <chrono>

double PI = PositionTrackerBase::PI;

void ZeroSensor(SensorsRaw& sensor)
{
    sensor.leftEncoder = 0;
    sensor.rightEncoder = 0;
    sensor.leftWheels = 0;
    sensor.rightWheels = 0;
    sensor.sideEncoder = 0;
};

/*******************************************************************************
 *
 * PositionTest class
 * 
 ******************************************************************************/
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
    double AccelerateStraight(double startingSpeed, double acceleration, unsigned int steps)
    {
        while (steps > 0)
        {
            startingSpeed += acceleration;
            m_posL += startingSpeed;
            m_posR += startingSpeed;

            Update();

            pros::c::delay(1);
            steps--;
        }
        return startingSpeed;
    }

    void MoveStraight(float acceleration, float accTime, float distance)
    {
        int moveTime = distance / acceleration / accTime -  accTime;
        AssertLess(0, moveTime);
        AccelerateStraight(0, acceleration, accTime);
        AccelerateStraight(acceleration * accTime, 0, moveTime);
        AccelerateStraight(acceleration * accTime, -acceleration, accTime);
    }

    double AccelerateArc(double R, double startingAngularVelocity, double angularAcceleration, unsigned int steps)
    {
        while (steps > 0)
        {
            startingAngularVelocity += angularAcceleration;
            m_posL += (R + DISTANCE_LR / 2) / TICKS_TO_IN_LR * startingAngularVelocity;
            m_posR += (R - DISTANCE_LR / 2) / TICKS_TO_IN_LR * startingAngularVelocity;
            m_angle += startingAngularVelocity;

            Update();

            pros::c::delay(1);
            steps--;
        }
        return startingAngularVelocity;
    }

    void MoveArc(float radius, float angle, unsigned int normalSpeedtime, unsigned int accTime)
    {
        float staringAngle = Angle();
        float speed = angle / normalSpeedtime;
        AccelerateArc(radius, 0, speed / accTime, accTime);
        int steps = (angle - 2 * (Angle() - staringAngle)) / speed;
        AssertLess(0, steps);
        AccelerateArc(radius, speed, 0, steps);
        AccelerateArc(radius, speed, -speed / accTime, accTime);
    }

    double PosL() const { return m_posL; }
    double PosR() const { return m_posR; }
    double Angle() const { return m_angle; }

    Position GetCoordinatesTicks()
    {
        auto pos = GetCoordinates();
        pos.X /= PositionTest::TICKS_TO_IN_LR;
        pos.Y /= PositionTest::TICKS_TO_IN_LR;
        return pos;
    }

    void PrintCoord()
    {
        auto pos = GetCoordinates();
        pos.X /= PositionTest::TICKS_TO_IN_LR;
        pos.Y /= PositionTest::TICKS_TO_IN_LR;
        printf("Coord: X,Y = (%f %f), a = %f\n", pos.X, pos.Y, pos.angle);
    }

protected:
    double m_posL = 0;
    double m_posR = 0;
    double m_angle = 0;
};


/*******************************************************************************
 *
 * Unit tests
 * 
 ******************************************************************************/
static Test testFast("Motion Fast", [] {
    PositionTest test;

    auto distance = 40 * 40 + 40 * 60;
    test.MoveStraight(1, 40, distance);
    test.AccelerateStraight(0, 0, 100);

    AssertEqual(test.PosR(), test.PosL());
    auto pos = test.GetCoordinatesTicks();
    AssertEqual(pos.X, 0);
    AssertLess(abs(pos.Y -test.PosL()), 0.0002);
    AssertLess(abs(pos.Y - distance), 0.0002);

    test.AccelerateStraight(0, 0, 100);
    auto pos2 = test.GetCoordinatesTicks();
    AssertEqual(pos.X, pos2.X);
    AssertLess(abs(pos.Y - pos2.Y), 0.0001);
});

static Test testSlow("Motion Slow backwards", [] {
    PositionTest test;

    float distance = - (0.002 * 500 * 500 + 1 * 2000);
    test.MoveStraight(-0.002, 500, distance);
    test.AccelerateStraight(0, 0, 10);

    AssertEqual(test.PosR(), test.PosL());
    auto pos = test.GetCoordinatesTicks();
    AssertEqual(pos.X, 0);
    AssertLess(abs(pos.Y - test.PosL()), 1);
    AssertLess(abs(pos.Y - distance), 1);
});

void testDiagonal(float angle, int sign = 1)
{
    PositionTest test;

    float distance = sign * (0.01 * 1000 * 1000 + 10 * 10000);
    test.SetAngle(angle);
    test.MoveStraight(sign * 0.01, 1000, distance);

    auto pos = test.GetCoordinatesTicks();
    angle *=  PI / 180;
    AssertLess(pos.X * sin(angle) * sign, 0);
    AssertLess(0, pos.Y * cos(angle) * sign);
    AssertLess(abs(pos.X + distance * sin(angle)), 1);
    AssertLess(abs(pos.Y - distance * cos(angle)), 1);
}

static Test testDiag30("Motion diagonally 30", [] { testDiagonal(30); });
static Test testDiag45("Motion diagonally 45", [] { testDiagonal(45); });
static Test testDiag60("Motion diagonally 60", [] { testDiagonal(60); });
static Test testDiag_30("Motion diagonally -30", [] { testDiagonal(-30); });
static Test testDiag120("Motion diagonally 120", [] { testDiagonal(90+30); });
static Test testDiag330("Motion diagonally 330", [] { testDiagonal(360-30); });

static Test testDiagBackwards1("Motion diagonally backwards 1", [] { testDiagonal(45, -1); });
static Test testDiagBackwards2("Motion diagonally backwards 2", [] { testDiagonal(-30, -1); });
static Test testDiagBackwards3("Motion diagonally backwards 3", [] { testDiagonal(90+30, -1); });
static Test testDiagBackwards4("Motion diagonally backwards 4", [] { testDiagonal(360-30, -1); });

void testRotation(float radius, int angleSign)
{
    PositionTest test;
    test.AccelerateArc(radius, angleSign * PI / 2000, 0, 1000);
    test.AccelerateArc(radius, 0, 0, 20);
    
    auto pos = test.GetCoordinatesTicks();
    AssertLess(abs(pos.angle + angleSign * 90), 0.1);
    AssertLess(abs(pos.X - angleSign * pos.Y), 1);
    AssertLess(abs(pos.X - radius / PositionTest::TICKS_TO_IN_LR), 0.5);
}

static Test testRotateUni1("Motion rotate uniform 1", [] { testRotation(15, 1); });
static Test testRotateUni2("Motion rotate uniform 2", [] { testRotation(15, -1); });
static Test testRotateUni3("Motion rotate uniform 3", [] { testRotation(-15, 1); });
static Test testRotateUni4("Motion rotate uniform 4", [] { testRotation(-15, -1); });

static Test testMoveArc("Motion rotate regular", [] {
    PositionTest test;
    float radius = 20;
    test.MoveArc(radius, PI / 2, 1000, 200);
    test.AccelerateArc(radius, 0, 0, 10);

    auto pos = test.GetCoordinatesTicks();
    AssertLess(abs(pos.angle + 90), 0.15);
    AssertLess(abs(pos.X - pos.Y), 2);
    AssertLess(abs(pos.X - radius / PositionTest::TICKS_TO_IN_LR), 2);
});

static Test testRotateCenter("Motion rotate center", [] {
    PositionTest test;

    test.SetCoordinates({.X = 100, .Y = 200, .angle = 0});

    for (int i = 0; i < 4; i++) {
        test.AccelerateArc(0, PI / 2000, 0, 1000);
        test.AccelerateArc(0, 0, 0, 20);

        auto pos = test.GetCoordinates();
        AssertLess(abs(pos.angle + 90 * (i+1)), 0.2);
        AssertLess(abs(pos.X - 100), 0.001);
        AssertLess(abs(pos.Y - 200), 0.001);
    }
});


/*******************************************************************************
 *
 * End-to-end test
 * 
 ******************************************************************************/
static Test testFull("End to end test", [] {
    PositionTest test;
    test.SetCoordinates({.X = 0, .Y = 0, .angle = 0});

    float duration1 = 2000;
    float duration2 = 2000;
    float angle = PI / 6;

    test.MoveStraight(0.002, 500, duration1);
    test.AccelerateStraight(0, 0, 20);

    test.MoveArc(0, angle, 1000, 200);
    test.AccelerateStraight(0, 0, 20);

    test.MoveStraight(0.002, 500, duration2);
    test.AccelerateStraight(0, 0, 20);

    test.MoveArc(0, -PI/2 - angle, 1000, 200);
    test.AccelerateStraight(0, 0, 20);

    test.MoveStraight(0.002, 500, duration2 / 2);
    test.AccelerateStraight(0, 0, 20);

    test.MoveArc(0, -PI / 2, 1000, 200);
    test.AccelerateStraight(0, 0, 20);

    test.MoveStraight(0.002, 500, duration1 + duration2 * sin(PI/3));
    test.AccelerateStraight(0, 0, 20);

    test.AccelerateStraight(0, 0, 20);
    test.PrintCoord();

    // TODO: Investigate how to improve accuracy here!
    auto pos = test.GetCoordinatesTicks();
    AssertLess(abs(AdjustAngle(pos.angle - 180)), 0.4);
    AssertLess(abs(pos.X), 14);
    AssertLess(abs(pos.Y), 2);
});


/*******************************************************************************
 *
 * Performance unit tests
 * 
 ******************************************************************************/
static Test speed("Motion calc speed", [] {
    PositionTest test;
    auto iterations = 2 * 1000 * 1000;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    test.AccelerateArc(0, PI / 2 / iterations, 0, iterations);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    auto itersPerMs = iterations * 1000.0 / time;
    // printf("Iterations per ms: %lld\n", (long long )itersPerMs);
    // This is very HW specific, but we should not see anything below 1000 on any more or less
    // modern PC / laptop (unless it's busy with other work)
    AssertLess(2000, itersPerMs);
});
