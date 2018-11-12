#include "drive.h"

float Drive::GetMovementJoystick(unsigned char joystick, unsigned char axis)
{
    int value = joystickGetAnalog(joystick, axis);
    if (abs(value) < 15)
    {
        value = 0;
    }
    if (value > 0)
    {
        return float(value * value) / -127;
    }
    else
    {
        return float(value * value) / 127;
    }
}
float Drive::GetForwardAxis()
{
    return GetMovementJoystick(1, 3);
}
float Drive::GetTurnAxis()
{
    return GetMovementJoystick(1, 1);
}

void Drive::SetLeftDrive(float speed)
{
    motorSet(leftDrivePortY, -speed);
    motorSet(leftDrivePort2, -speed);
}

void Drive::SetRightDrive(float speed)
{
    motorSet(rightDrivePortY, speed);
    motorSet(rightDrivePort2, speed);
}

void Drive::Update()
{
    //Drive
    forward = GetForwardAxis();
    turn = GetTurnAxis();
    if (turn == 0 && forward == 0)
    {

        SetLeftDrive(0);
        SetRightDrive(0);
    }
    else if (turn == 0)
    {

        float k = 0.1;

        SetLeftDrive(forward - ErrorPower);
        SetRightDrive(forward + ErrorPower);

        int error = encoderGet(leftDriveEncoder) - encoderGet(rightDriveEncoder);

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