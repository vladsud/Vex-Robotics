#include "shooter.h"

bool Shooter::GetShooter()
{
    return joystickGetDigital(1, 7, JOY_LEFT);
}

void Shooter::SetShooterMotor(float speed)
{
    motorSet(shooterPort, speed);
}

void Shooter::Update()
{
    m_shooter = GetShooter();

    if (m_shooter)
    {
        SetShooterMotor(shooterMotorSpeed);
    }
    else
    {
        SetShooterMotor(0);
    }
}