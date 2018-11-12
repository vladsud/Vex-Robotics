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
    motorSet(liftPort, speed);
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