#include "drive.h"
#include "gyro.h"

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

float Drive::GetTurnAxis()
{
    // We adjust speed to put more power to motors initially to move robot from still position.
    if (isAuto())
        return m_overrideTurn;
    // convert joystick metric to drive metric
    return -GetMovementJoystick(1, 1, 30) * driveMotorMaxSpeed / joystickMax;
}

void Drive::OverrideInputs(int forward, float turn)
{
    Assert(isAuto());

    bool keepDirection = KeepDrection(forward, turn);
    // Are you sure in what you do??? Direction will be reset any way
    m_overrideForward = forward;
    m_overrideTurn = turn;

    printf("OverrideInputs: %d %d %d %d %d\n", int(m_overrideForward), int(m_overrideTurn), int(m_forward), int(m_turn), (int)keepDirection);

    // We need this to properly count turning on the spot without using gyro.
    // Maybe this will not be needed in the future).
    if (!keepDirection)
        ResetEncoders();
}

void Drive::ResetEncoders()
{
    encoderReset(g_leftDriveEncoder);
    encoderReset(g_rightDriveEncoder);
    m_distance = 0;
    m_distanceTurn = 0;
    m_ErrorIntergral = 0;
    m_gyro = GyroWrapper::Get();
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

void Drive::Update()
{
    // DebugDrive();

    //Drive
    int forward = GetForwardAxis();
    int turn = GetTurnAxis();

    bool keepDirection = KeepDrection(forward, turn); 

    if (!keepDirection)
        ResetEncoders();
    // 400 is roughtly one full turn, positive is forward
    int left = encoderGet(g_leftDriveEncoder);
    int right = encoderGet(g_rightDriveEncoder);

    m_distance = abs(left) + abs(right);
    m_distanceTurn = left - right;

    // Do not reset m_forward & m_turn on full stop!
    // that screws up autonomous turning.
    if (turn == 0 && forward == 0)
    {
        SetLeftDrive(0);
        SetRightDrive(0);
        return;
    }

    m_forward = forward;
    m_turn = turn;

    bool smartsOn = SmartsOn();

    float error;
    if (true)
    {
        // turn is aded to left!
        // when m_distance = 2*m_forward, we expect left-right to be 2*turn 
        if (abs(m_turn) < abs(m_forward))
            error  =  left - right - m_turn * m_distance / abs(m_forward);
        else if (m_forward == 0)
            error  =  left + right;
        else
            smartsOn = false;
    }
    else
    {
        // 256 ticks is one degree.
        // We expect turn = 10 to be one degree turn over one cycle (2*forward)
        // Gyro moves positive counter-clock-wise.
        // gyroDiff > 0: clock-wise movement
        int gyroDiff = m_gyro - GyroWrapper::Get();
        error = (gyroDiff - m_turn * m_distance / m_forward * GyroWrapper::Multiplier / 20) / GyroWrapper::Multiplier;
    }

    if (!smartsOn)
    {
        // if we are not moving forward, then we want to put all power to motors to turn
        // But if we are moving forward 100%, we do not want to completely stop one motor if
        // turning 100% to the right - we still want to make forward progress! 
        int turn = m_turn * (0.6 + 0.4 * abs(m_forward) / driveMotorMaxSpeed);

        SetLeftDrive(m_forward + turn);
        SetRightDrive(m_forward - turn);
        return;
    }

    m_ErrorIntergral += error;

    int errorMultiplier = error * (2 + abs(error) * 0.5) + m_ErrorIntergral * 0.1;


    // if we were going forward for a while and then started turning slightly, then there is huge amount of inertia
    // If we allow unbounded adjustments, then speed of noe motor will drop to zero and below because of this interia,
    // causing stop and jerking. Not something we want!
    int turnAbs = abs(int(m_turn));
    int forwardAbs = abs(m_forward);
    if (forwardAbs > turnAbs)
    {
        int maxAdjustment = (forwardAbs + turnAbs) / 2;
        if (errorMultiplier > maxAdjustment)
            errorMultiplier = maxAdjustment;
        else if (errorMultiplier < -maxAdjustment)
            errorMultiplier = -maxAdjustment;
    }

    // adjust power on slower motor
    // positive error with positive forward - left is going too fast
    // negative error with negative forward - left is going too fast
    int leftAdjustment = 0;
    int rightAdjustment = 0;
    if (m_forward * error > 0)
        leftAdjustment = errorMultiplier;
    else
        rightAdjustment = errorMultiplier;

    /*
    printf("Drive: encoders: (%d, %d), erorr: (%d, %d), integral: %d, Distance: %d, turn/dist/forw: %d, Gyro diff: %d, Speeds (%d, %d)\n",
            left, right, int(error), errorMultiplier, int(m_ErrorIntergral), m_distance, int(m_turn * m_distance / m_forward),
            m_gyro - GyroWrapper::Get(),
            int(m_forward - leftAdjustment + m_turn),
            int(m_forward + rightAdjustment - m_turn));
            */
    SetLeftDrive(m_forward - leftAdjustment + m_turn);
    SetRightDrive(m_forward + rightAdjustment - m_turn);
}
