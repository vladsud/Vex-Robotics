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
    liftUp = GetLiftUp();
    liftDown = GetLiftDown();

    if (liftUp)
    {
        SetLiftMotor(-liftMotorSpeed);
    }
    else if (liftDown)
    {
        SetLiftMotor(liftMotorSpeed / 2);
    }
    else
    {
        SetLiftMotor(0);
    }
}