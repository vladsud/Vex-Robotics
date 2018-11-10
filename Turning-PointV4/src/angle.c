#include "main.h"

bool angle = false;
bool angleUp = false;
bool angleDown = false;


bool GetAngleUp()
{
	return joystickGetDigital(1, 7, JOY_UP);
}

bool GetAngleDown()
{
	return joystickGetDigital(1, 7, JOY_DOWN);
}

void SetAngleMotor(float speed)
{
	motorSet(ANGLEPORT, speed);
}

void UpdateAngle()
{
	angleUp = GetAngleUp();
	angleDown = GetAngleDown();

	if (angleUp)
	{
		SetAngleMotor(ANGLEMOTORSPEED);
	}
	else if (angleDown)
	{
		SetAngleMotor(-ANGLEMOTORSPEED / 2);
	}
	else
	{
		SetAngleMotor(0);
	}
}
