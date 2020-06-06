#include "main.h"
#include "position.h"
#include "pros/motors.h"

/*******************************************************************************
 * 
 * PositionTracker
 * 
*******************************************************************************/
PositionTracker::PositionTracker()
{
}

void PositionTracker::ReadSensors(SensorsRaw& sensor)
{
    sensor.leftEncoder = m_leftEncoder.GetPos(); //  motor_get_position(leftBackDrivePort);
    sensor.rightEncoder = m_rightEncoder.GetPos(); //motor_get_position(rightBackDrivePort);
    sensor.sideEncoder = 0;
}

Position PositionTracker::GetCoordinates()
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
    float angle = PositionTrackerBase::GetAngle();
    if (m_flipX)
        angle = -angle;
    return angle;
}

void PositionTracker::SetAngle(int angle)
{
    if (m_flipX)
        angle = -angle;
    PositionTrackerBase::SetAngle(angle);
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

    PositionTrackerBase::ResetState();
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
CMotor::CMotor(unsigned int port, bool reverse)
    : m_port(port)
{
    pros::c::motor_set_reversed(m_port, reverse);
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

