#include "main.h"
#include "position.h"
#include <cmath>
#include "pros/motors.h"

/*******************************************************************************
 * 
 * PositionTracker
 * 
*******************************************************************************/
PositionTracker::PositionTracker()
{
    motor_set_reversed(rightBackDrivePort, true);
    motor_set_reversed(rightFrontDrivePort, true);
}

void PositionTracker::ReadSensors(Sensors& sensor)
{
    sensor.leftEncoder = m_leftEncoder.GetPos(); //  motor_get_position(leftBackDrivePort);
    sensor.rightEncoder = m_rightEncoder.GetPos(); //motor_get_position(rightBackDrivePort);
    sensor.sideEncoder = 0;
    sensor.rightWheels = m_motorRightBack.GetPos();
    sensor.leftWheels = m_motorLeftBack.GetPos();

    // float angle = GetGyro().GetAngle() * AngleToRadiants;
    sensor.angle = (sensor.rightEncoder - sensor.leftEncoder) * TICKS_TO_IN_LR / DISTANCE_LR;
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

int Encoder::GetPos() const
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

int CMotor::GetPos() const
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

