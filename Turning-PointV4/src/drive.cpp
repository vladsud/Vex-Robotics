#include "drive.h"

int AdjustSpeed(int speed)
{
    // motors can't move robot at slow speed, so add some boost
    // We start withputting cutoff/2 = 20 power, and slowly (at half speed) increasing it to cutoff = 40.
    // After that it's linnier 
    const int cutoff = 30;

    if (speed == 0)
        return 0;
    bool neg = speed < 0;
    speed = abs(speed);
    if (speed < cutoff)
        speed = (speed + cutoff) / 2;
    if (speed > driveMotorMaxSpeed)
        speed = driveMotorMaxSpeed;
    return neg ? - speed : speed;
}

int Drive::GetMovementJoystick(unsigned char joystick, unsigned char axis)
{
    int value = joystickGetAnalog(joystick, axis);

    // dead zone on joystick - it can get stuck there even without finger
    // note: 10 isnot enough!
    bool neg = value < 0;
    value = abs(value);

    const int cutoff = 15;
    if (value <= cutoff)
        return 0;

    /*
    if (value > 0)
    {
        return -float(value * value) / joystickMax;
    }
    else
    {
        return float(value * value) / joystickMax;
    }
    */

   // Linear function that fills in range 0..joystickMax 
    value = (value - cutoff) * joystickMax / (joystickMax - cutoff);
    return neg ? value : -value;
}

int Drive::GetForwardAxis()
{
    return GetMovementJoystick(1, 3);
}

int Drive::GetTurnAxis()
{
    // We adjust speed (see AdjustSpeed()) to put more power to motors initially to move robot from still position.
    // That's not enough when turning, because of more friction.
    // So need to increase the difference slghtly between rigth & left initially.
    // NOte: this adjustment is combined with speed adjustment in AdjustMOtor(), so cutoff is much smaller. 
    const int cutoff = 10;
    int turn = GetMovementJoystick(1, 1);
    if (turn == 0)
        return 0;
    if (turn > 0 && turn < cutoff)
        return (turn + cutoff) / 2;
    if (turn < 0 && turn > -cutoff)
        return (turn - cutoff) / 2;
    return turn;
}

void Drive::SetLeftDrive(int speed)
{
    speed = AdjustSpeed(speed);
    motorSet(leftDrivePortY, -speed);
    motorSet(leftDrivePort2, -speed);
}

void Drive::SetRightDrive(int speed)
{
    speed = AdjustSpeed(speed);
    motorSet(rightDrivePortY, speed);
    motorSet(rightDrivePort2, speed);
}

void Drive::DebugDrive()
{
    // will be used for debugging in the future...
}

void Drive::Update()
{
    DebugDrive();

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
        int right = -encoderGet(rightDriveEncoder);
        int error =  left - right;
        // int speed = left + right;

        m_ErrorPower = error * 0.1;

        SetLeftDrive(m_forward - m_ErrorPower);
        SetRightDrive(m_forward + m_ErrorPower);
    }
    else
    {
        // convert joystick metric to drive metric
        m_turn = m_turn * driveMotorMaxSpeed / joystickMax;

        // if we are not moving forward, then we want to put all power to motors to turn
        // But if we are moving forward 100%, we do not want to completely stop one motor if
        // turning 100% to the right - we still want to make forward progress! 
        m_turn *= (1 - abs(m_forward) / driveMotorMaxSpeed / 2);

        SetLeftDrive(m_forward + m_turn);
        SetRightDrive(m_forward - m_turn);
    }

    encoderReset(leftDriveEncoder);
    encoderReset(rightDriveEncoder);
}