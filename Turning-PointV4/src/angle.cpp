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
    angleUp = GetAngleUp();
    angleDown = GetAngleDown();

    if (angleUp)
    {
        SetAngleMotor(angleMotorSpeed);
    }
    else if (angleDown)
    {
        SetAngleMotor(-angleMotorSpeed / 2);
    }
    else
    {
        SetAngleMotor(0);
    }
}