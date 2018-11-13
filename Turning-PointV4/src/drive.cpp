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
        return -float(value * value) / 127;
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
    // We adjust speed (see AdjustSpeed()) to put more power to motors initially to move robot from still position.
    // That's not enough when turning, because of more friction.
    // So need to increase the difference slghtly between rigth & left initially.
    // NOte: this adjustment is combined with speed adjustment in AdjustMOtor(), so cutoff is much smaller. 
    const int cutoff = 15;
    float turn = GetMovementJoystick(1, 1);
    if (turn == 0)
        return 0;
    if (turn > 0 && turn < cutoff)
        return (turn + cutoff) / 2;
    if (turn < 0 && turn > -cutoff)
        return (turn - cutoff) / 2;
    return turn;
}

int AdjustSped(int speed)
{
    // motors can't move robot at slow speed, so add some boost
    // We start withputting cutoff/2 = 20 power, and slowly (at half speed) increasing it to cutoff = 40.
    // After that it's linnier 
    const int cutoff = 40;

    if (speed == 0)
        return 0;
    if (speed > 0 && speed < cutoff)
        return (speed + cutoff) / 2;
    if (speed < 0 && speed > -cutoff)
        return (speed - cutoff) / 2;
    return speed;
}


void Drive::SetLeftDrive(int speed)
{
    speed = AdjustSped(speed);
    motorSet(leftDrivePortY, -speed);
    motorSet(leftDrivePort2, -speed);
}

void Drive::SetRightDrive(int speed)
{
    speed = AdjustSped(speed);
    motorSet(rightDrivePortY, speed);
    motorSet(rightDrivePort2, speed);
}

void Drive::Update()
{
    //Drive
    m_forward = GetForwardAxis();
    m_turn = GetTurnAxis();
    if (m_turn == 0 && m_forward == 0)
    {
        SetLeftDrive(0);
        SetRightDrive(0);
    }
    else if (m_turn == 0)
    {
        int left = encoderGet(leftDriveEncoder);
        int right = encoderGet(rightDriveEncoder);
        int error =  left - right;
        // int speed = left + right;

        m_ErrorPower = error * 0.1;

        SetLeftDrive(m_forward - m_ErrorPower);
        SetRightDrive(m_forward + m_ErrorPower);
    }
    else
    {
        encoderReset(leftDriveEncoder);
        encoderReset(rightDriveEncoder);

        SetLeftDrive(m_forward + m_turn);
        SetRightDrive(m_forward - m_turn);
    }
}