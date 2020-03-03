#include "drive.h"
#include "lcd.h"
#include "position.h"

using namespace pros;
using namespace pros::c;


/*******************************************************************************
 *
 * Standalone functions
 * 
 ******************************************************************************/
int AdjustSpeed(int speed)
{
    if (speed > MotorMaxSpeed)
        speed = MotorMaxSpeed;
    if (speed <= -MotorMaxSpeed)
        speed = -MotorMaxSpeed;
    return speed;
}

int GetMovementJoystick(pros::controller_id_e_t joystick, pros::controller_analog_e_t axis, int minValue)
{
    int value = controller_get_analog(joystick, axis);

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


/*******************************************************************************
 *
 * DriveTracker class
 * 
 ******************************************************************************/
DriveTracker::DriveTracker()
    : m_drive(GetDrive())
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


/*******************************************************************************
 *
 * KeepAngle class
 * 
 ******************************************************************************/
KeepAngle::KeepAngle(int angle)
{
    if (m_drive.IsXFlipped())
        angle = -angle;

    AssertSz(abs(GetError()) <= 10, "Angle is too far from current one!");
    m_angle = angle;
}

float KeepAngle::GetError()
{
    auto angle = GetTracker().GetAngle();
    if (m_drive.IsXFlipped())
        angle = -angle;

    return (m_angle - angle) * 2.0;
}


/*******************************************************************************
 *
 * Drive class
 * 
 ******************************************************************************/
Drive::Drive()
{
}

int Drive::GetForwardLeftAxis()
{
    Assert(!isAuto());
    // motors can't move robot at slow speed, so add some boost
    return -GetMovementJoystick(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_LEFT_Y, 18);
}

int Drive::GetForwardRightAxis()
{
    Assert(!isAuto());
    // motors can't move robot at slow speed, so add some boost
    return -GetMovementJoystick(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_RIGHT_Y, 18);
}

int Drive::GetForward()
{
    if (isAuto())
        return m_overrideForward;
    if (GetLcd().IsTankDrive)
        return (GetForwardLeftAxis() + GetForwardRightAxis())/2;
    return GetForwardLeftAxis();
}

int Drive::GetTurnAxis()
{
    // We adjust speed to put more power to motors initially to move robot from still position.
    if (isAuto())
        return m_overrideTurn;
    if (GetLcd().IsTankDrive)
        return (GetForwardLeftAxis() - GetForwardRightAxis())/2;
    // convert joystick metric to drive metric
    return -GetMovementJoystick(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_RIGHT_X, 25) * MotorMaxSpeed / joystickMax;
}

void Drive::OverrideInputs(int forward, int turn)
{
    Assert(isAuto());

    if (m_flipX)
        turn = -turn;

    m_overrideForward = forward;
    m_overrideTurn = turn;
}

void Drive::ResetState()
{
    m_tracker = nullptr;
    ResetTrackingState();
}

void Drive::ResetTrackingState()
{
    auto& tracker = GetTracker();
    m_encoderLeftBase = tracker.GetLeftPos();
    m_encoderRightBase = tracker.GetRightPos();
    m_ErrorIntergral = 0;
}

void setMotors(uint8_t forwardPort, uint8_t backPort, int speed)
{
    /*
    const int speedLimitOnReverse = 20;
    speed = AdjustSpeed(speed);
    if (speed < 0 && motor_get_actual_velocity(forwardPort) > 70)
    {
        motor_move(forwardPort, max(speed, -1));
        motor_move(backPort, speed);
    }
    else if (speed > 0 && motor_get_actual_velocity(backPort) < -20)
    {
        motor_move(backPort, min(speed, 5));
        motor_move(forwardPort, speed);
    }
    else
    */
    {
        motor_move(backPort, AdjustSpeed(speed));
        motor_move(forwardPort, AdjustSpeed(speed));
    }
} 

void Drive::SetLeftDrive(int speed)
{
    setMotors(leftFrontDrivePort, leftBackDrivePort, speed);
}

void Drive::SetRightDrive(int speed)
{
    setMotors(rightFrontDrivePort, rightBackDrivePort, speed);
}

int Drive::GetLeft() {
    return GetTracker().GetLeftPos() - m_encoderLeftBase;
}

int Drive::GetRight() {
    return GetTracker().GetRightPos() - m_encoderRightBase;
}

int Drive::GetDistance()
{
    return GetLeft() + GetRight();
}

void Drive::Update()
{
    // Drive
    const int forward = GetForward();
    const int turn = GetTurnAxis();
    
    // It's hard to make this work in automnomous mode, as sometimes direcitons change as part of single attion.
    // So each autonomous action resets state manually
    if (!isAuto() && !KeepDrection(forward, turn))
        ResetTrackingState();

    // Remember direction of movement!
    if (turn != 0 || forward != 0)
    {
        m_forward = forward;
        m_turn = turn;
    }

    int turnAbs = abs(turn);
    int forwardAbs = abs(forward);

    bool smartsOn = SmartsOn();

    float error;
    // turn is aded to left!
    // when GetDistance() = 2*forward, we expect left-right to be 2*turn
    if (m_tracker)
    {
        // tracker is used in very limited cases
        Assert(turnAbs == 0);
        Assert(isAuto());
        Assert(smartsOn);

        // Gyro moves positive counter-clock-wise.
        // gyroDiff > 0: clock-wise movement
        float gyroDiff = m_tracker->GetError();
        if (forward != 0)
            gyroDiff -= turn * abs(GetDistance()) / forward;
        error = gyroDiff;
    }
    else if (turnAbs < forwardAbs || (turn == 0 && forward == 0 && m_forward != 0))
    {
        error = GetLeft() - GetRight();
        if (forwardAbs != 0)
            error -= turn * abs(GetDistance()) / forwardAbs;
    }
    else if (forward == 0)
        error = GetLeft() + GetRight();
    else
        smartsOn = false;

    if (!smartsOn)
    {
        SetLeftDrive(forward + turn);
        SetRightDrive(forward - turn);
        return;
    }

    m_ErrorIntergral += error;

    // power is non-linear!
    int errorMultiplier;
    if (forwardAbs > 90)
        errorMultiplier = error * (40 + 4 * abs(error)) / 20; // + m_ErrorIntergral * 0.1;
    else if (forwardAbs > 50)
        errorMultiplier = error * (40 + 8 * abs(error)) / 20; // + m_ErrorIntergral * 0.1;
    else
        errorMultiplier = error * (40 + 8 * abs(error)) / 20; // + m_ErrorIntergral * 0.1;

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
        if (turn * errorMultiplier > 0)
            leftAdjustment = errorMultiplier;
        else
            rightAdjustment = -errorMultiplier;
    }

    int leftMotor = forward - leftAdjustment + turn;
    int rightMotor = forward + rightAdjustment - turn;

    SetLeftDrive(leftMotor);
    SetRightDrive(rightMotor);

    ReportStatus(Log::Drive, "error = %d, errInt = %d, left = %d, right = %d\n",
            int(error), errorMultiplier,
            leftMotor,
            rightMotor);
}
