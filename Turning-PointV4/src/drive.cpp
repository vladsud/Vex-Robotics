#include "drive.h"

int AdjustSpeed(int speed)
{
    if (speed > driveMotorMaxSpeed)
        speed = driveMotorMaxSpeed;
    if (speed <= -driveMotorMaxSpeed)
        speed = -driveMotorMaxSpeed;
    return speed;
}

int Drive::GetMovementJoystick(unsigned char joystick, unsigned char axis, int minValue)
{
    int value = joystickGetAnalog(joystick, axis);

    bool neg = value < 0;
    value = abs(value);

    // dead zone on joystick - it can get stuck there even without finger
    // note: 20 is not enough!
    const int cutoff = 23;
    if (value <= cutoff)
        return 0;

    // Linear function that fills in range 0..(joystickMax-minValue)
    value = (value - cutoff) * (joystickMax - minValue) / (joystickMax - cutoff);

    value = value * value / (joystickMax - minValue) + minValue;
    return neg ? value : -value;
}

int Drive::GetForwardAxis()
{
    // motors can't move robot at slow speed, so add some boost
    if (isAuto())
        return m_overrideForward;
    return -GetMovementJoystick(1, 3, 18);
}

int Drive::GetTurnAxis()
{
    // We adjust speed to put more power to motors initially to move robot from still position.
    if (isAuto())
        return m_overrideTurn;
    // convert joystick metric to drive metric
    return -GetMovementJoystick(1, 1, 30) * driveMotorMaxSpeed / joystickMax;
}

void Drive::OverrideInputs(int forward, int turn)
{
    Assert(isAuto());

    m_overrideForward = forward;
    m_overrideTurn = turn;

    // We need this to properly count turning on the spot without using gyro.
    // Maybe this will not be needed in the future).
    ResetEncoders();
}

void Drive::ResetEncoders()
{
    encoderReset(g_leftDriveEncoder);
    encoderReset(g_rightDriveEncoder);
    m_distance = 0;
    m_ErrorIntergral = 0;
}

void Drive::SetLeftDrive(int speed)
{
    speed = AdjustSpeed(speed);
    motorSet(leftDrivePortY, speed);
    motorSet(leftDrivePort2, speed);
}

void Drive::SetRightDrive(int speed)
{
    speed = AdjustSpeed(speed);
    motorSet(rightDrivePortY, -speed);
    motorSet(rightDrivePort2, -speed);
}

void Drive::DebugDrive()
{
    // will be used for debugging in the future...
}

bool SmartsOn()
{
    return true;
    return isAuto();
}

void Drive::Update()
{
    // DebugDrive();

    //Drive
    int forward = GetForwardAxis();
    int turn = GetTurnAxis();

    bool resetDirection = (m_turn != turn || m_forward != forward);

    m_forward = forward;
    m_turn = turn;

    if (resetDirection)
    {
        ResetEncoders();
        m_ErrorIntergral = 0;
    }

    // 400 is roughtly one full turn, positive is forward
    int left = encoderGet(g_leftDriveEncoder);
    int right = encoderGet(g_rightDriveEncoder);

    m_distance = (abs(left) + abs(right)) / 2;

    if (m_turn == 0 && m_forward == 0)
    {
        SetLeftDrive(0);
        SetRightDrive(0);
        return;
    }

    bool smartsOn = SmartsOn();

    float error;
    if (abs(m_turn) < abs(m_forward))
        error  =  left - right - 2 * m_turn * m_distance / m_forward;
    else if (m_forward == 0)
        error  =  left + right;
    else
        smartsOn = false;

    if (!smartsOn)
    {
        // if we are not moving forward, then we want to put all power to motors to turn
        // But if we are moving forward 100%, we do not want to completely stop one motor if
        // turning 100% to the right - we still want to make forward progress! 
        m_turn = m_turn * (0.6 + 0.4 * abs(m_forward) / driveMotorMaxSpeed);

        SetLeftDrive(m_forward + m_turn);
        SetRightDrive(m_forward - m_turn);
        return;
    }

    m_ErrorIntergral += error;

    if (error > 0)
    {
        error += 4;
        if (error > 10)
            error = 10;
    }
    else if (error < 0)
    {
        error -= 4;
        if (error < -10)
            error = -10;
    }
        
    int errorMultiplier = error * (0.3 + abs(error) * 0.1) + m_ErrorIntergral * 0.02;
    
    // adjust power on slower motor
    int leftAdjustment = 0;
    int rightAdjustment = 0;
    if (m_forward * error > 0)
        rightAdjustment = errorMultiplier;
    else
        leftAdjustment = errorMultiplier;

    SetLeftDrive(m_forward - leftAdjustment + m_turn);
    SetRightDrive(m_forward + rightAdjustment - m_turn);
}
