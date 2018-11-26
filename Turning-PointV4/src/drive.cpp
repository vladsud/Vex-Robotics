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
    return GetMovementJoystick(1, 3, 18);
}

int Drive::GetTurnAxis()
{
    // We adjust speed to put more power to motors initially to move robot from still position.
    return -GetMovementJoystick(1, 1, 30);
}

void Drive::SetLeftDrive(int speed)
{
    m_LeftSpeed = speed;
    speed = AdjustSpeed(speed);
    motorSet(leftDrivePortY, speed);
    motorSet(leftDrivePort2, speed);
}

void Drive::SetRightDrive(int speed)
{
    m_RightSpeed = speed;
    speed = AdjustSpeed(speed);
    motorSet(rightDrivePortY, -speed);
    motorSet(rightDrivePort2, -speed);
}

void Drive::DebugDrive()
{
    // will be used for debugging in the future...
    encoderReset(g_leftDriveEncoder);
    encoderReset(g_rightDriveEncoder);

    m_forward = 90;

    delay(2000);

    while (true)
    {
        // 400 is roughtly one full turn, positive is forward
        int left = encoderGet(g_leftDriveEncoder);
        int right = encoderGet(g_rightDriveEncoder);

        int error =  left - right;
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
            
        m_ErrorPowerLeft = 0;
        m_ErrorPowerRight = 0;

        int errorMultiplier = error * (0.7 + abs(error) * 0.15) + m_ErrorIntergral * 0.05;
        // reduce power on faster motor
        if (m_forward * error > 0)
            m_ErrorPowerLeft = (m_ErrorPowerLeft + errorMultiplier) / 2;
        else
            m_ErrorPowerRight = (m_ErrorPowerRight + errorMultiplier) / 2;

        SetLeftDrive(m_forward - m_ErrorPowerLeft);
        SetRightDrive(m_forward + m_ErrorPowerRight);

        printf("Diff: %d, integral: %d, Reading: %d, %d, Drive: %d, %d\n", left - right, m_ErrorIntergral, left, right, m_forward - m_ErrorPowerLeft, m_forward + m_ErrorPowerRight);

        delay(10);
    }
}

bool SmartsOn()
{
    // return isAutonomous();
    return false;
}

void Drive::Update()
{
    // DebugDrive();

    bool wasForward = (m_turn == 0 && m_forward != 0);

    //Drive
    m_forward = -GetForwardAxis();
    m_turn = GetTurnAxis();

    if (m_turn == 0 && m_forward == 0)
    {
        SetLeftDrive(0);
        SetRightDrive(0);
    }
    else if (m_turn == 0)
    {
        if (!SmartsOn())
        {
            SetLeftDrive(m_forward);
            SetRightDrive(m_forward);
        }
        else
        {
            if (!wasForward)
            {
                encoderReset(g_leftDriveEncoder);
                encoderReset(g_rightDriveEncoder);
                m_ErrorIntergral = 0;
            }

            // 400 is roughtly one full turn, positive is forward
            int left = encoderGet(g_leftDriveEncoder);
            int right = encoderGet(g_rightDriveEncoder);

            int error =  left - right;
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
                
            m_ErrorPowerLeft = 0;
            m_ErrorPowerRight = 0;

            int errorMultiplier = error * (0.7 + abs(error) * 0.15) + m_ErrorIntergral * 0.05;
            // reduce power on faster motor
            if (m_forward * error > 0)
                m_ErrorPowerLeft = (m_ErrorPowerLeft + errorMultiplier) / 2;
            else
                m_ErrorPowerRight = (m_ErrorPowerRight + errorMultiplier) / 2;

            SetLeftDrive(m_forward - m_ErrorPowerLeft);
            SetRightDrive(m_forward + m_ErrorPowerRight);
        } 
    }
    else
    {
        // convert joystick metric to drive metric
        m_turn = m_turn * driveMotorMaxSpeed / joystickMax;

        // if we are not moving forward, then we want to put all power to motors to turn
        // But if we are moving forward 100%, we do not want to completely stop one motor if
        // turning 100% to the right - we still want to make forward progress! 
        m_turn = m_turn * (0.6 + 0.4 * abs(m_forward) / driveMotorMaxSpeed);

        SetLeftDrive(m_forward + m_turn);
        SetRightDrive(m_forward - m_turn);
    }
}


void AutoDriveForward(float distance, int power) {
    encoderReset(leftDriveEncoder);
    encoderReset(rightDriveEncoder);

    float leftMotorPower = 100;
    float rightMotorPower = 100;

    float error = 0;
    float K = 0.1

    while (encoderGet(leftDriveEncoder) < distance){
        SetLeftDrive(leftMotorPower);
        SetRightDrive(rightMotorPower);

        error = (encoderGet(leftDriveEncoder) - encoderGet(rightDriveEncoder)) * K;

        leftMotorPower += error;
        rightMotorPower -= error;

    }

    SetLeftDrive(0);
    SetRightDrive(0);

}