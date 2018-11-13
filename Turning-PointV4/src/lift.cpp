#include "lift.h"

bool Lift::GetLiftUp()
{
    return joystickGetDigital(1, 5, JOY_UP);
}

bool Lift::GetLiftDown()
{
    return joystickGetDigital(1, 5, JOY_DOWN);
}

void Lift::SetLiftMotor(float speed)
{
    // slowly decrease speed when going up.
    if (speed == 0 && m_LiftSpeed < 0)
    {
        if (m_LiftSpeed < -100)
            m_LiftSpeed = -100;
        m_LiftSpeed = 0.97 * m_LiftSpeed + 0.03 * speed;
    }
    else
        m_LiftSpeed = speed;
    motorSet(liftPort, m_LiftSpeed);
}

void Lift::Update()
{
    m_liftUp = GetLiftUp();
    m_liftDown = GetLiftDown();

    if (m_liftUp)
    {
        SetLiftMotor(-liftMotorSpeed);
    }
    else if (m_liftDown)
    {
        SetLiftMotor(liftMotorSpeed / 2);
    }
    else
    {
        SetLiftMotor(0);
    }
}