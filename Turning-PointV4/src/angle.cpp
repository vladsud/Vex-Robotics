#include "angle.h"

bool Angle::GetAngleUp()
{
    return joystickGetDigital(1, 7, JOY_UP);
}

bool Angle::GetAngleDown()
{
    return joystickGetDigital(1, 7, JOY_DOWN);
}

void Angle::SetAngleMotor(float speed)
{
    motorSet(anglePort, speed);
}

void Angle::Update()
{
    m_angleUp = GetAngleUp();
    m_angleDown = GetAngleDown();

    if (m_angleUp)
    {
        SetAngleMotor(angleMotorSpeed);
    }
    else if (m_angleDown)
    {
        SetAngleMotor(-angleMotorSpeed / 2);
    }
    else
    {
        SetAngleMotor(0);
    }
}