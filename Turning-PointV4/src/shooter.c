#include "main.h"

bool shooter = false;

bool GetShooter()
{
	return joystickGetDigital(1, 8, JOY_UP);
}
void SetShooterMotor(float speed)
{
	motorSet(SHOOTERPORT, -speed);
}

void UpdateShooter()
{
	shooter = GetShooter();

	if (shooter)
	{
		SetShooterMotor(SHOOTERMOTORSPEED);
	}
	else
	{
		SetShooterMotor(0);
	}
}