#include "drive.h"
#include "position.h"
#include "logger.h"
#include "cycle.h"
#include <math.h>

int AdjustSpeed(int speed)
{
    if (speed > driveMotorMaxSpeed)
        speed = driveMotorMaxSpeed;
    if (speed <= -driveMotorMaxSpeed)
        speed = -driveMotorMaxSpeed;
    return speed;
}


DriveTracker::DriveTracker()
    : m_main(GetMain()),
      m_drive(m_main.drive)
{
    Assert(isAuto());
    m_drive.ResetTrackingState();
    m_drive.StartTracking(this);
}

DriveTracker::~DriveTracker()
{
    Assert(isAuto());
    m_drive.StopTracking();
}


KeepAngle::KeepAngle(int angle)
{
    if (m_drive.IsXFlipped())
        angle = -angle;
    angle *= GyroWrapper::Multiplier;

    AssertSz(abs(angle - GetGyro().Get()) <= 20 * GyroWrapper::Multiplier, "Angle is too far from current one!");
    m_angle = angle;
}

float KeepAngle::GetError()
{
    return (m_angle - GetGyro().Get()) * 5.0 / GyroWrapper::Multiplier;
}


int Drive::GetMovementJoystick(unsigned char joystick, unsigned char axis, int minValue)
{
    int value = joystickGetAnalog(joystick, axis);

    bool neg = value < 0;
    value = abs(value);

    // dead zone on joystick - it can get stuck there even without finger
    // note: 20 is not enough!
    const int cutoff = 25;
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
    return -GetMovementJoystick(1, 1, 37) * driveMotorMaxSpeed / joystickMax;
}

void Drive::OverrideInputs(int forward, float turn)
{
    Assert(isAuto());

    if (m_flipX)
        turn = -turn;

    m_overrideForward = forward;
    m_overrideTurn = turn;

    if (PrintDiagnostics(Diagnostics::Drive))
        ReportStatus("OverrideInputs: %d %d %d %d\n", int(m_overrideForward), int(m_overrideTurn), int(m_forward), int(m_turn));
}

void Drive::ResetState()
{
    m_tracker = nullptr;
    m_distanceFromBeginning = 0;
    encoderReset(g_leftDriveEncoder);
    encoderReset(g_rightDriveEncoder);
    encoderReset(g_sideEncoder);
    ResetTrackingState();
}

void Drive::ResetTrackingState()
{
    m_encoderBaseLeft = encoderGet(g_leftDriveEncoder);
    m_encoderBaseRight = encoderGet(g_rightDriveEncoder);
    m_distance = 0;
    m_left = 0;
    m_right = 0;
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

void Drive::StartHoldingPosition()
{
    m_holdingPosition = true;
    ResetTrackingState();
}

void Drive::HoldPosition()
{
    int left = encoderGet(g_leftDriveEncoder) - m_encoderBaseLeft;
    int right = encoderGet(g_rightDriveEncoder) - m_encoderBaseRight;
    PositionInfo info = GetTracker().LatestPosition(false /*clicks*/);

    if (left * info.leftSpeed >= 0)
        SetLeftDrive(-left * 127);
    else
        SetLeftDrive(-30 * Sign(left) - left - info.leftSpeed * 40);

    if (right * info.rightSpeed >= 0)
        SetLeftDrive(-right * 127);
    else
        SetLeftDrive(-30 * Sign(right) - right - info.rightSpeed * 40);
}

void Drive::UpdateDistanes()
{
    m_left = encoderGet(g_leftDriveEncoder);
    m_right = encoderGet(g_rightDriveEncoder);
    m_distanceFromBeginning = abs(m_left) + abs(m_right);

    m_left -= m_encoderBaseLeft;
    m_right -= m_encoderBaseRight;
    m_distance = abs(m_left) + abs(m_right);    
}

void Drive::Update()
{
    //Drive
    int forward = GetForwardAxis();
    int turn = GetTurnAxis();

    if (m_holdingPosition)
    {
        if (forward == 0 && turn == 0)
        {
            HoldPosition();
            return;
        }
        m_holdingPosition = false;
    }

    // It's hard to make this work in automnomous mode, as sometimes direcitons change as part of single attion.
    // So each autonomous action resets state manually
    bool keepDirection = isAuto() || KeepDrection(forward, turn);

    if (!keepDirection)
        ResetTrackingState();

    UpdateDistanes();

    if (turn == 0 && forward == 0)
    {
        SetLeftDrive(0);
        SetRightDrive(0);
        return;
    }

    m_forward = forward;
    m_turn = turn;

    int turnAbs = abs(m_turn);
    int forwardAbs = abs(m_forward);

    bool smartsOn = SmartsOn();

    float error;
    // turn is aded to left!
    // when m_distance = 2*m_forward, we expect left-right to be 2*turn
    Assert(m_tracker == nullptr || (turnAbs == 0 && smartsOn && isAuto())); // tracker is used in very limited cases
    if (turnAbs < forwardAbs)
    {
        // Use gyro only in limited cases, until proven to be working well.
        if (turnAbs == 0 && m_tracker != nullptr)
        {
            // Gyro moves positive counter-clock-wise.
            // gyroDiff > 0: clock-wise movement
            float gyroDiff = m_tracker->GetError();
            error = gyroDiff - m_turn * m_distance / m_forward;
        }
        else
        {
            error = m_left - m_right - m_turn * m_distance / forwardAbs;
        }
    }
    else if (m_forward == 0)
        error = m_left + m_right;
    else
        smartsOn = false;

    if (!smartsOn)
    {
        // if we are not moving forward, then we want to put all power to motors to turn
        // But if we are moving forward 100%, we do not want to completely stop one motor if
        // turning 100% to the right - we still want to make forward progress!
        int turn = m_turn * (0.6 + 0.4 * forwardAbs / driveMotorMaxSpeed);

        SetLeftDrive(m_forward + turn);
        SetRightDrive(m_forward - turn);
        return;
    }

    m_ErrorIntergral += error;

    // power is non-linear!
    int errorMultiplier;
    if (forwardAbs > 90)
        errorMultiplier = error * (40 + 8 * abs(error)) / 20; // + m_ErrorIntergral * 0.1;
    else if (forwardAbs > 60)
        errorMultiplier = error * (20 + 4 * abs(error)) / 20; // + m_ErrorIntergral * 0.1;
    else
        errorMultiplier = error * (10 + abs(error)) / 20; // + m_ErrorIntergral * 0.1;

    // if we were going forward for a while and then started turning slightly, then there is huge amount of inertia
    // If we allow unbounded adjustments, then speed of one motor will drop to zero and below because of this interia,
    // causing stop and jerking. Not something we want!
    int maxAdjustment = forwardAbs / 2 + turnAbs / 5;
    if (errorMultiplier > maxAdjustment)
        errorMultiplier = maxAdjustment;
    else if (errorMultiplier < -maxAdjustment)
        errorMultiplier = -maxAdjustment;

    int leftAdjustment = 0;
    int rightAdjustment = 0;
    if (forward != 0)
    {
        // adjust power on slower motor
        // positive error with positive forward - left is going too fast
        // negative error with negative forward - left is going too fast
        if (forward * errorMultiplier > 0)
            leftAdjustment = errorMultiplier;
        else
            rightAdjustment = errorMultiplier;
    }
    else
    {
        if (m_turn * errorMultiplier > 0)
            leftAdjustment = errorMultiplier;
        else
            rightAdjustment = -errorMultiplier;
    }

    int leftMotor = m_forward - leftAdjustment + m_turn;
    int rightMotor = m_forward + rightAdjustment - m_turn;

    SetLeftDrive(leftMotor);
    SetRightDrive(rightMotor);

    if (PrintDiagnostics(Diagnostics::Drive))
    {
        ReportStatus("Drive: gyro: %d, erorr: (%d, %d), Speeds (%d, %d)\n",
               // left, right,
               GetGyro().Get() * 10 / GyroWrapper::Multiplier,
               int(error), errorMultiplier,
               // int(m_ErrorIntergral),
               // m_distance, int(m_turn * m_distance / m_forward),
               leftMotor,
               rightMotor);
    }
}
