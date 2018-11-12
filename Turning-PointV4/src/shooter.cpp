#include "shooter.h"

bool Shooter::GetShooter()
{
    return joystickGetDigital(1, 8, JOY_UP);
}
void Shooter::SetShooterMotor(float speed)
{
    motorSet(shooterPort, -speed);
}
void Shooter::Update()
{
    shooter = GetShooter();

    if (shooter)
    {
        SetShooterMotor(shooterMotorSpeed);
    }
    else
    {
        SetShooterMotor(0);
    }
}