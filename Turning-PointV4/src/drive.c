#include "main.h"

float forward = 0;
float turn = 0;

float ErrorPower = 0;

float GetMovementJoystick(unsigned char joystick, unsigned char axis)
{
	float value = joystickGetAnalog(joystick, axis);
	if (abs(value) < 15)
	{
		value = 0;
	}
	if (value > 0)
	{
		value = (value * value) / -127;
	}
	else
	{
		value = (value * value) / 127;
	}
	return value;
}
float GetForwardAxis()
{
	return GetMovementJoystick(1, 3);
}
float GetTurnAxis()
{
	return GetMovementJoystick(1, 1);
}

void SetLeftDrive(float speed)
{
	motorSet(LEFTDRIVEPORTY, -speed);
	motorSet(LEFTDRIVEPORT2, -speed);
}
void SetRightDrive(float speed)
{
	motorSet(RIGHTDRIVEPORTY, speed);
	motorSet(RIGHTDRIVEPORT2, speed);
}

void UpdateDrive()
{

	forward = GetForwardAxis();
	turn = GetTurnAxis();
	if (turn == 0 && forward == 0)
	{

		SetLeftDrive(0);
		SetRightDrive(0);
	}
	else if (turn == 0)
	{

		int error = 0;
		float k = 0.1;
		SetLeftDrive(forward - ErrorPower);
		SetRightDrive(forward + ErrorPower);
		error = encoderGet(leftDriveEncoder) - encoderGet(rightDriveEncoder);

		ErrorPower = error * k;
	}
	else
	{
		encoderReset(leftDriveEncoder);
		encoderReset(rightDriveEncoder);
		SetLeftDrive(forward + turn);
		SetRightDrive(forward - turn);
	}
}