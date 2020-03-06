#include "main.h"
#include "position.h"
#include <cmath>
#include "pros/motors.h"

/*******************************************************************************
 * 
 * PositionTracker
 * 
*******************************************************************************/
void PositionTracker::ReadSensors(Sensors& sensor)
{
    sensor.leftEncoder = m_leftEncoder.GetPos(); //  motor_get_position(leftBackDrivePort);
    sensor.rightEncoder = m_rightEncoder.GetPos(); //motor_get_position(rightBackDrivePort);
    sensor.sideEncoder = 0;
    sensor.rightWheels = m_motorRightBack.GetPos();
    sensor.leftWheels = m_motorLeftBack.GetPos();

    // float angle = GetGyro().GetAngle() * AngleToRadiants;
    sensor.angle = (sensor.rightEncoder - sensor.leftEncoder) * SPIN_TO_IN_LR / DISTANCE_LR;
}


void PositionTracker::UpdateSensorSpeed(const Sensors& pos, SensorSpeed& speed, unsigned int timeDiff)
{
    int delta = millis() - speed.time;
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

void PositionTracker::SynthesizeSensors(const Sensors& pos, const SensorSpeed& speed, Sensors& posOut)
{
    posOut.leftWheels = pos.leftWheels;
    posOut.rightWheels = pos.rightWheels;
    posOut.sideEncoder = pos.sideEncoder;

    float alpha = 0.9;
    posOut.leftEncoder = (1 - alpha) * (posOut.leftEncoder + speed.leftEncoder) + alpha * pos.leftEncoder;
    posOut.rightEncoder = (1 - alpha) * (posOut.rightEncoder + speed.rightEncoder) + alpha * pos.rightEncoder;

    posOut.angle = (posOut.rightEncoder - posOut.leftEncoder) * SPIN_TO_IN_LR / DISTANCE_LR;
}

void PositionTracker::InitSensorSpeed(const Sensors& pos, SensorSpeed& speed)
{
    m_sensorSpeed.time = millis();
    speed.last = pos;
    UpdateSensorSpeed(pos, speed, 0);
}

PositionTracker::PositionTracker()
{
    motor_set_reversed(rightBackDrivePort, true);
    motor_set_reversed(rightFrontDrivePort, true);

    ReadSensors(m_sensorsRaw);
    InitSensorSpeed(m_sensorsRaw, m_sensorSpeedRaw);
    m_sensors = m_sensorsRaw;

    InitSensorSpeed(m_sensors, m_sensorSpeed);
    InitSensorSpeed(m_sensors, m_sensorSpeedSlow);

    m_position.X = 0;
    m_position.Y = 0;
    m_position.angle = m_sensors.angle;

    Update();
}

// Based on http://thepilons.ca/wp-content/uploads/2018/10/Tracking.pdf
// We are missing here third wheel, so we can't account for bumps form the side.
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
void PositionTracker::Update()
{
    ReadSensors(m_sensorsRaw);
    UpdateSensorSpeed(m_sensorsRaw, m_sensorSpeedRaw, 0);
    SynthesizeSensors(m_sensorsRaw, m_sensorSpeed, m_sensors);

    UpdateSensorSpeed(m_sensors, m_sensorSpeed, 0);
    UpdateSensorSpeed(m_sensors, m_sensorSpeedSlow, 10);

    auto angle = m_sensors.angle;
    auto deltaLeft  = m_sensorSpeed.leftEncoder * SPIN_TO_IN_LR;
    auto deltaRight = m_sensorSpeed.rightEncoder * SPIN_TO_IN_LR;
    auto deltaSide  = m_sensorSpeed.sideEncoder * SPIN_TO_IN_S;
    auto deltaAngle  = m_sensorSpeed.angle;

    auto arcMoveForward = (deltaLeft + deltaRight) / 2;

    auto angleDiffHalf = deltaAngle / 2;
    if (deltaAngle != 0)
    {
        auto sinDiffHalf = sin(angleDiffHalf);
        auto coeff = sinDiffHalf / angleDiffHalf;
        arcMoveForward *= coeff ;

        deltaSide *= coeff;
        deltaSide -= 2 * sinDiffHalf * DISTANCE_S;
    }

    auto angleHalf = angle - angleDiffHalf;
    auto sinA = sin(angleHalf);
    auto cosA = cos(angleHalf);

    m_position.X -= sinA * arcMoveForward;
    m_position.Y += cosA * arcMoveForward;

    m_position.X += cosA * deltaSide;
    m_position.Y += sinA * deltaSide;

    m_position.angle = m_sensors.angle;

    m_count++;

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

Position PositionTracker::LatestPosition()
{
    Position info = m_position;
    info.angle = GetAngle();
    if (m_flipX)
        info.X = -info.X;
    return info;
}

void PositionTracker::FlipX(bool flip)
{
    m_flipX = flip;
}

float PositionTracker::GetAngle()
{
    float angle = m_position.angle / AngleToRadiants;
    if (m_flipX)
        angle = -angle;
    return angle + m_angleOffset;
}

void PositionTracker::SetAngle(int angle)
{
    m_angleOffset = 0;
    m_angleOffset = angle - GetAngle();
    Assert(GetAngle() == angle);
}

void PositionTracker::SetCoordinates(Position coord)
{
    SetAngle(coord.angle);

    if (m_flipX)
        coord.X = -coord.X;

    m_position.X = coord.X;
    m_position.Y = coord.Y;
}


void PositionTracker::ResetState()
{
    m_motorLeftBack.HardReset();
    m_motorRightBack.HardReset();
    m_motorLeftFront.HardReset();
    m_motorRightFront.HardReset();
    m_leftEncoder.HardReset();
    m_rightEncoder.HardReset();
}

float PositionTracker::GetRobotVelocity()
{
    return m_sensorSpeedSlow.leftEncoder + m_sensorSpeedSlow.rightEncoder;
}

int PositionTracker::GetLeftPos()
{
    return m_sensors.leftEncoder;
}

int PositionTracker::GetRightPos()
{
    return m_sensors.rightEncoder;
}

/*******************************************************************************
 *
 * Encoder class
 * 
 ******************************************************************************/
Encoder::Encoder(std::uint8_t portTop, std::uint8_t portBottom, bool reverse)
{
    m_encoder = adi_encoder_init(portTop - 'A' + 1, portBottom - 'A' + 1, reverse);
    Assert(m_encoder != PROS_ERR);
    HardReset();
}

void Encoder::HardReset()
{
    adi_encoder_reset(m_encoder);
}

int Encoder::GetPos()
{
    return adi_encoder_get(m_encoder);
}


/*******************************************************************************
 *
 * CMotor class
 * 
 ******************************************************************************/
CMotor::CMotor(unsigned int port)
    : m_port(port)
{
    HardReset();
}

void CMotor::HardReset()
{
    motor_tare_position(m_port);
    motor_set_encoder_units(m_port, pros::E_MOTOR_ENCODER_COUNTS);
}

int CMotor::GetPos()
{
    return motor_get_position(m_port);
}


/*******************************************************************************
 *
 * GetGyroReading()
 * 
 ******************************************************************************/
float GetGyroReading()
{
    return GetTracker().GetAngle();
}


/*******************************************************************************
 *
 * PidImpl::GetPower
 * 
 ******************************************************************************/
int PidImpl::GetPower(int reading, int target, float kp, int ki, PidPrecision precision /*= PidPrecision:Precise */)
{
    if (target != m_target)
    {
        m_target = target;
        m_errorAccumulated = 0;
    }

    int error = reading - target;

    if (error > 0 && precision == PidPrecision::HigherOk ||
        error < 0 && precision == PidPrecision::LowerOk ||
        (abs(error) <= m_precision))
    {
        m_errorAccumulated = 0;
        return 0;
    }

    m_errorAccumulated += error;
    float res = (float) error / kp + m_errorAccumulated / ki;
    if (res > MotorMaxSpeed)
        res = MotorMaxSpeed;
    else if (res < -MotorMaxSpeed)
        res = -MotorMaxSpeed;
    return res;
}

