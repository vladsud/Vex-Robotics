#include "positionCore.h"

extern "C" {
   double sin(double);
   double cos(double);
}

// double atan2(double, double);

/*******************************************************************************
 * 
 * Helper methods
 * 
*******************************************************************************/
template <typename T>
void InitSensorSpeed(const T& pos, SensorSpeed<T>& speed)
{
    speed.time = pros::c::millis();
    speed.last = pos;
    UpdateSensorSpeed(pos, speed, 0);
}

template <typename T>
void UpdateSensorSpeed(const T& pos, SensorSpeed<T>& speed, unsigned int timeDiff)
{
    int delta = pros::c::millis() - speed.time;
    Assert(delta >= 0);
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

void SynthesizeSensors(const SensorsRaw& pos, const SensorSpeed<SensorsRaw>& speed, Sensors& posOut)
{
    posOut.leftWheels = pos.leftWheels;
    posOut.rightWheels = pos.rightWheels;
    posOut.sideEncoder = pos.sideEncoder;

    // Experimentally, 0.5 gives one of the better results in terms of precision
    float alpha = 0.5;
    posOut.leftEncoder = (1 - alpha) * (posOut.leftEncoder + speed.leftEncoder) + alpha * pos.leftEncoder;
    posOut.rightEncoder = (1 - alpha) * (posOut.rightEncoder + speed.rightEncoder) + alpha * pos.rightEncoder;

    posOut.angle = (posOut.rightEncoder - posOut.leftEncoder) * PositionTrackerBase::TICKS_TO_IN_LR / PositionTrackerBase::DISTANCE_LR;
}


/*******************************************************************************
 * 
 * PositionTrackerBase
 * 
*******************************************************************************/
PositionTrackerBase::PositionTrackerBase()
{
    ResetState();
}

void PositionTrackerBase::ResetState()
{
    SensorsRaw raw {};
    InitSensorSpeed(raw, m_sensorSpeedSlow);

    m_sensors = {};
    InitSensorSpeed(m_sensors, m_sensorDelta);

    m_position.X = 0;
    m_position.Y = 0;
    m_position.angle = m_sensors.angle;

    m_angleOffset = 0;
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
    // Raw data from sensors
    SensorsRaw sensorsRaw {};
    ReadSensors(sensorsRaw);

    // Experimentally, calculating speed over 10..40 ms seems to give best results, with 20 being sweet spot
    UpdateSensorSpeed(sensorsRaw, m_sensorSpeedSlow, 20);

    SynthesizeSensors(sensorsRaw, m_sensorSpeedSlow, m_sensors);

    UpdateSensorSpeed(m_sensors, m_sensorDelta, 0);
 
    auto angle = m_sensors.angle + m_angleOffset;
    auto deltaAngle  = m_sensorDelta.angle;

    auto arcMoveForward = (m_sensorDelta.leftEncoder + m_sensorDelta.rightEncoder) * TICKS_TO_IN_LR / 2;
    auto deltaSide  = m_sensorDelta.sideEncoder * TICKS_TO_IN_S;

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

    m_position.angle = angle;

    /*
    static int count = 0;
    count++;
    if ((count % 2000) == 0)
        printf("Angle = %f, X,Y = (%f, %f), encoders = (%d %d)\n",
            sensorsRaw.angle / AngleToRadiants,
            m_position.X,
            m_position.Y,
            sensorsRaw.leftEncoder,
            sensorsRaw.rightEncoder);
    */
}

Position PositionTrackerBase::GetCoordinates()
{
    Position info = m_position;
    info.angle = GetAngle();
    return info;
}

void PositionTrackerBase::SetCoordinates(Position coord)
{
    SetAngle(coord.angle);
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

double PositionTrackerBase::GetAngle()
{
    return (m_position.angle + m_angleOffset) / AngleToRadiants;
}

void PositionTrackerBase::SetAngle(float angle)
{
    m_angleOffset = angle * AngleToRadiants - m_position.angle;
}
