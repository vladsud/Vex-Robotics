#include "drive.h"
#include "position.h"
#include "lcd.h"
#include "actions.h"

#include "pros/motors.h"
#include "pros/misc.h"

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
 * Motor class
 * 
 ******************************************************************************/
Motor::Motor(unsigned int port)
    : m_port(port)
{
    HardReset();
}

void Motor::HardReset()
{
    motor_tare_position(m_port);
    motor_set_encoder_units(m_port, pros::E_MOTOR_ENCODER_COUNTS);
    Reset();
    Update();
    m_prevValue = m_currValue;
}

void Motor::Reset()
{
    m_base = motor_get_position(m_port);
}

void Motor::Update()
{
    m_prevValue = m_currValue;
    m_currValue = motor_get_position(m_port);
}

int Motor::GetRealTimePos()
{
    return motor_get_position(m_port) - m_base;
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

    AssertSz(abs(angle - GetGyro().GetAngle()) <= 5, "Angle is too far from current one!");
    m_angle = angle;
}

float KeepAngle::GetError()
{
    return (m_angle - GetGyro().GetAngle()) * 2.0;
}


/*******************************************************************************
 *
 * Drive class
 * 
 ******************************************************************************/
int Drive::GetFrontVelocity()
{
    return m_motorRightFront.GetVelocity() + m_motorLeftFront.GetVelocity();
}

int Drive::GetBackVelocity()
{
    return m_motorRightBack.GetVelocity() + m_motorLeftBack.GetVelocity();
}

int Drive::GetRobotVelocity()
{
    return (
        GetFrontVelocity() +
        GetBackVelocity()
    ) / 2;
}

Drive::Drive()
  : m_motorLeftFront(leftFrontDrivePort)
  , m_motorLeftBack(leftBackDrivePort)
  , m_motorRightFront(rightFrontDrivePort)
  , m_motorRightBack(rightBackDrivePort)
{
    motor_set_reversed(rightBackDrivePort, true);
    motor_set_reversed(rightFrontDrivePort, true);
    ResetState();
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
    m_motorLeftBack.HardReset();
    m_motorRightBack.HardReset();
    m_motorLeftFront.HardReset();
    m_motorRightFront.HardReset();
    ResetTrackingState();
}

void Drive::ResetTrackingState()
{
    m_motorLeftBack.Reset();
    m_motorRightBack.Reset();
    m_motorLeftFront.Reset();
    m_motorRightFront.Reset();
    m_distance = 0;
    m_left = 0;
    m_right = 0;
    m_ErrorIntergral = 0;
}

void setMotors(uint8_t forwardPort, uint8_t backPort, int speed)
{
    const int speedLimitOnReverse = 20;

    speed = AdjustSpeed(speed);
    /*
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
        motor_move(backPort, speed);
        motor_move(forwardPort, speed);
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

int Drive::GetRealTimeDistance()
{
    return m_motorLeftBack.GetRealTimePos() + m_motorRightBack.GetRealTimePos();
}


int Drive::GetAngle()
{
    return m_motorRightBack.GetRawPos() - m_motorLeftBack.GetRawPos();
}

// This method has to run before Update() in each update cycle!
void Drive::UpdateOdometry()
{
    m_motorLeftFront.Update();
    m_motorLeftBack.Update();
    m_motorRightFront.Update();
    m_motorRightBack.Update();
    m_left = m_motorLeftBack.GetPos();
    m_right = m_motorRightBack.GetPos();
    m_distance = m_left + m_right;
}

void Drive::Update()
{
    //Drive
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
        error = m_left - m_right;
        if (forwardAbs != 0)
            error -= turn * abs(GetDistance()) / forwardAbs;
    }
    else if (forward == 0)
        error = m_left + m_right;
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

struct WaitTillStopsAction : public Action
{
    bool ShouldStop() override
    {
        return abs(GetDrive().GetRobotVelocity()) <= 2;
    }
    const char* Name() override { return "WaitTillStopsAction"; } 
};

// give some time for robot to completely stop
void WaitAfterMove(unsigned int timeout /*= 0*/)
{
    /*
    // Not enough time in "main" atonomous
    auto& lcd = GetLcd();
    if (timeout == 0)
        timeout = lcd.AtonSkills || !lcd.AtonProtected || !lcd.AtonClimbPlatform ? 500 : 200;
    */
    Do(WaitTillStopsAction(), timeout == 0 ? 200 : timeout);
}
