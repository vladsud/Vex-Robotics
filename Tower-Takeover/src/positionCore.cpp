#include "positionCore.h"

extern "C" {
   double sin(double);
   double cos(double);
}

// float cos (float x);
// float sin (float x);
float atan2 (float y, float x);
double atan2(double, double);


PositionTrackerBase::PositionTrackerBase()
{
    InitSensorSpeed(m_sensors, m_sensorSpeed);
    InitSensorSpeed(m_sensors, m_sensorSpeedSlow);

    m_position.X = 0;
    m_position.Y = 0;
    m_position.angle = m_sensors.angle;
}

void PositionTrackerBase::InitSensorSpeed(const Sensors& pos, SensorSpeed& speed)
{
    speed.time = pros::c::millis();
    speed.last = pos;
    UpdateSensorSpeed(pos, speed, 0);
}

void PositionTrackerBase::UpdateSensorSpeed(const Sensors& pos, SensorSpeed& speed, unsigned int timeDiff)
{
    int delta = pros::c::millis() - speed.time;
    if (delta < timeDiff)
        return;
    speed.time += delta;
    if (delta == 0)
        delta = 1;
    speed.angle = float (pos.angle - speed.last.angle) / delta;
    speed.leftEncoder = float (pos.leftEncoder - speed.last.leftEncoder) / delta;
    speed.rightEncoder = float (pos.rightEncoder - speed.last.rightEncoder) / delta;
    speed.sideEncoder = float (pos.sideEncoder - speed.last.sideEncoder) / delta;
    speed.leftWheels = float (pos.leftWheels - speed.last.leftWheels) / delta;
    speed.rightWheels = float (pos.rightWheels - speed.last.rightWheels) / delta;
    speed.last = pos;
}

void PositionTrackerBase::SynthesizeSensors(const Sensors& pos, const SensorSpeed& speed, Sensors& posOut)
{
    posOut.leftWheels = pos.leftWheels;
    posOut.rightWheels = pos.rightWheels;
    posOut.sideEncoder = pos.sideEncoder;

    float alpha = 0.9;
    posOut.leftEncoder = (1 - alpha) * (posOut.leftEncoder + speed.leftEncoder) + alpha * pos.leftEncoder;
    posOut.rightEncoder = (1 - alpha) * (posOut.rightEncoder + speed.rightEncoder) + alpha * pos.rightEncoder;

    posOut.angle = (posOut.rightEncoder - posOut.leftEncoder) * TICKS_TO_IN_LR / DISTANCE_LR;
}

// Based on http://thepilons.ca/wp-content/uploads/2018/10/Tracking.pdf
// We are missing here third wheel, so we can't account for bumps form the side.
// Also see http://www.hmc.edu/lair/ARW/ARW-Lecture01-Odometry.pdf
//
// ***         ***
// ***  TURNS  ***
// ***         ***
//      Full in-place turn == 360*3.547 clicks on each wheel.
//      Or roughly 0.14 degrees per one click.
//      Gyro, on another hand, 0.004 degree resolution (or 35.5 times better than wheel resolution).
//      Max full turn in one second == 360 degree/second, measurements per 10ms:
//          - 25.5 click difference between wheels
//          - Or 256*3.6 = 921.6 clicks on gyro.
//
// ***         ***
// ***  MOVING ***
// ***         ***
//      One tick is roughly 0.9 mm
//      Assuming 24" / second speed:
//              670 clicks / second
//              6.7 clicks per 10ms
//      Assuming extreame case (which is not possible)
//          1 click / millisecond = 35" / second
//      Tthat said, we exceeed that rate when turning, as robot is not really moving (less energy spent per click)
//
void PositionTrackerBase::Update()
{
    ReadSensors(m_sensorsRaw);
    SynthesizeSensors(m_sensorsRaw, m_sensorSpeed, m_sensors);

    UpdateSensorSpeed(m_sensors, m_sensorSpeed, 0);
    UpdateSensorSpeed(m_sensors, m_sensorSpeedSlow, 10);

    auto angle = m_sensors.angle;
    auto deltaAngle  = m_sensorSpeed.angle;

    auto arcMoveForward = (m_sensorSpeed.leftEncoder + m_sensorSpeed.rightEncoder) * TICKS_TO_IN_LR / 2;
    auto deltaSide  = m_sensorSpeed.sideEncoder * TICKS_TO_IN_S;

    auto angleDiffHalf = deltaAngle / 2;
    if (deltaAngle != 0)
    {
        auto sinDiffHalf = sin(angleDiffHalf);
        auto coeff = sinDiffHalf / angleDiffHalf;
        arcMoveForward *= coeff ;

        deltaSide *= coeff;
        deltaSide -= 2 * sinDiffHalf * DISTANCE_S;
    }

    auto angleMiddle = angle - angleDiffHalf;
    auto sinA = sin(angleMiddle);
    auto cosA = cos(angleMiddle);

    m_position.X -= sinA * arcMoveForward;
    m_position.Y += cosA * arcMoveForward;

    m_position.X += cosA * deltaSide;
    m_position.Y += sinA * deltaSide;

    m_position.angle = m_sensors.angle;

    /*
    static int count = 0;
    count++;
    if ((count % 2000) == 0)
        printf("Angle = %f, X,Y = (%f, %f), encoders = (%d %d)\n",
            m_sensorsRaw.angle / AngleToRadiants,
            m_position.X,
            m_position.Y,
            m_sensorsRaw.leftEncoder,
            m_sensorsRaw.rightEncoder);
    */
}

Position PositionTrackerBase::LatestPosition()
{
    Position info = m_position;
    info.angle = m_position.angle / AngleToRadiants;
    return info;
}

void PositionTrackerBase::SetCoordinates(Position coord)
{
    m_position.angle = coord.angle * AngleToRadiants;
    m_position.X = coord.X;
    m_position.Y = coord.Y;
}

float PositionTrackerBase::GetRobotVelocity()
{
    return m_sensorSpeedSlow.leftEncoder + m_sensorSpeedSlow.rightEncoder;
}

int PositionTrackerBase::GetLeftPos()
{
    return m_sensors.leftEncoder;
}

int PositionTrackerBase::GetRightPos()
{
    return m_sensors.rightEncoder;
}
