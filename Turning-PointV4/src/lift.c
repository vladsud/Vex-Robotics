#include "main.h"

bool GetLiftUp()
{
	return joystickGetDigital(1, 5, JOY_UP);
}

bool GetLiftDown()
{
	return joystickGetDigital(1, 5, JOY_DOWN);
}

void SetLiftMotor(float speed)
{
	motorSet(LIFTPORT, speed);
}

bool liftUp = false;
bool liftDown = false;

void UpdateLift()
{
	liftUp = GetLiftUp();
	liftDown = GetLiftDown();

	if (liftUp)
	{
		SetLiftMotor(-LIFTMOTORSPEED);
	}
	else if (liftDown)
	{
		SetLiftMotor(LIFTMOTORSPEED / 2);
	}
	else
	{
		SetLiftMotor(0);
	}
}