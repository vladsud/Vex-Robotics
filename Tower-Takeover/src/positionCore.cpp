#include "positionCore.h"

extern "C" {
   double sin(double);
   double cos(double);
}

// double atan2(double, double);

/*******************************************************************************
 * 
 * Stand-alone functions
 * 
 ******************************************************************************/
float AdjustAngle(float angle)
{
    while (angle > 180)
        angle -= 360;
    while (angle < -180)
        angle += 360;
    return angle;
}

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
    if (timeDiff == 0)
        delta = 1;
    speed.angle = float (pos.angle - speed.last.angle) / delta;
    speed.leftEncoder = float (pos.leftEncoder - speed.last.leftEncoder) / delta;
    speed.rightEncoder = float (pos.rightEncoder - speed.last.rightEncoder) / delta;
    speed.sideEncoder = float (pos.sideEncoder - speed.last.sideEncoder) / delta;
    speed.last = pos;
}

void SynthesizeSensors(const SensorsRaw& pos, Sensors& posOut)
{
    posOut.sideEncoder = pos.sideEncoder;

    // NOTE: Experimentally, this gives us 50ms latency in data!!!
    float alpha = 0.125;
    posOut.leftEncoder = (1 - alpha) * posOut.leftEncoder + alpha * pos.leftEncoder;
    posOut.rightEncoder = (1 - alpha) * posOut.rightEncoder + alpha * pos.rightEncoder;

    posOut.angle = (posOut.rightEncoder - posOut.leftEncoder) * PositionTrackerBase::TICKS_TO_IN_LR / PositionTrackerBase::DISTANCE_LR;
}


/*******************************************************************************
 * 
 * PositionTrackerBase
 * 
*******************************************************************************/
PositionTrackerBase::PositionTrackerBase()
{
	m_gyro.ResetState();
    ResetState();
}

void PositionTrackerBase::ResetState()
{
    m_lastUpdated = pros::c::millis();
    m_sensorsRaw = {};

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
// One tick is roughly
//     - 0.6 mm of motion (both wheels)
//     - 0.14 degrees of turning (single wheel, gyro has 35x better precision)
// Max speeeds:
//     - 360 rotaton / second: 2571 clicks wheel difference
//     - 24" / second motion: 988 clicks / sec / wheel
//
void PositionTrackerBase::Update()
{
	m_gyro.Integrate();

    // Raw data from sensors
    SensorsRaw sensorsRaw {};
    ReadSensors(sensorsRaw);
    
    unsigned int time = pros::c::millis();

    if (time -  m_lastUpdated < 10 &&
        sensorsRaw.leftEncoder == m_sensorsRaw.leftEncoder &&
        sensorsRaw.rightEncoder == m_sensorsRaw.rightEncoder &&
        sensorsRaw.sideEncoder == m_sensorsRaw.sideEncoder)
        return;

    const int badReading = 16*10254*1024;
    if (abs(sensorsRaw.leftEncoder) > badReading ||
        abs(sensorsRaw.rightEncoder) > badReading ||
        abs(sensorsRaw.sideEncoder) > badReading)
    {
        return;        
    }

    m_lastUpdated = time;

    m_sensorsRaw = sensorsRaw;

    SynthesizeSensors(sensorsRaw, m_sensors);

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

    ReportStatus(Log::Position, "Angle = %f, %f XY = %f, %f, enc = %d, %d\n",
        m_position.angle / AngleToRadiants,
        m_gyro.GetAngle(),
        m_position.X,
        m_position.Y,
        sensorsRaw.leftEncoder,
        sensorsRaw.rightEncoder);

    /*
    static int count = 0;
    count++;
    if ((count % 200) == 0)
        printf("Angle = %f, %f, XY = (%f, %f), enc = (%d, %d)\n",
            m_position.angle / AngleToRadiants,
            m_gyro.GetAngle(),
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
    return m_sensorDelta.leftEncoder + m_sensorDelta.rightEncoder;
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

void PositionTrackerBase::PrintPos(Log logCategory)
{
    ReportStatus(logCategory, "Angle = %f, %f XY = %f, %f, enc = %d, %d\n",
        m_position.angle / AngleToRadiants,
        m_gyro.GetAngle(),
        m_position.X,
        m_position.Y,
        m_sensorsRaw.leftEncoder,
        m_sensorsRaw.rightEncoder);
}