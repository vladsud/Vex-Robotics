#include "shooter.h"
#include "logger.h"
#include <math.h>
#include <cstdio>
#include "pros/adi.h"
#include "pros/motors.h"
#include "pros/rtos.h"
#include <limits.h>
#include "cycle.h"

using namespace pros;
using namespace pros::c;

const unsigned int distanceFirstAton = 54; // main shot, 2 balls
const unsigned int distanceFirstAtonDiagonalShot = 72; // medium flag
const unsigned int distanceFirstAtonFromPlatform = 90; // medium flag near platform
const unsigned int distanceSecondAton = 100;        // high, then medium

// Distance based on front of the robot
constexpr float Distances[]             {48,  54,  90, 100};
constexpr unsigned int AnglesHigh[]   { 410, 350, 410, 330};
constexpr unsigned int AnglesMedium[] { 80, 30, 240, 1};

constexpr unsigned int LastDistanceCount = CountOf(Distances) - 1;

constexpr unsigned int ConvertAngleToPotentiometer(unsigned int angle)
{
    return 700-angle;
}


StaticAssert(CountOf(Distances) == CountOf(AnglesHigh));
StaticAssert(CountOf(Distances) == CountOf(AnglesMedium));

StaticAssert(Distances[0] < Distances[1]);
StaticAssert(Distances[1] < Distances[2]);
StaticAssert(Distances[2] < Distances[3]);

StaticAssert(AnglesHigh[0] >= AnglesHigh[1]);
// StaticAssert(AnglesHigh[1] >= AnglesHigh[2]);

StaticAssert(AnglesMedium[0] >= AnglesMedium[1]);
// StaticAssert(AnglesMedium[1] >= AnglesMedium[2]);

StaticAssert(AnglesMedium[0] < AnglesHigh[0]);
StaticAssert(AnglesMedium[1] < AnglesHigh[1]);
StaticAssert(AnglesMedium[2] < AnglesHigh[2]);
StaticAssert(AnglesMedium[3] < AnglesHigh[3]);

constexpr unsigned int CalcAngle(Flag flag, float distanceInches)
{
    const unsigned int *angles = (flag == Flag::High) ? AnglesHigh : AnglesMedium;
    unsigned int count = CountOf(Distances);

    if (Distances[0] >= distanceInches)
        return angles[0];

    for (unsigned int i = 0; i < count; i++)
    {
        if (Distances[i] >= distanceInches)
        {
            return angles[i - 1] - (distanceInches - Distances[i - 1]) * int(angles[i - 1] - angles[i]) / (Distances[i] - Distances[i - 1]);
        }
    }

    return angles[count - 1];
}

constexpr bool AlmostSameAngle(unsigned int angle1, unsigned int angle2)
{
    return angle1 == angle2;
}


StaticAssert(CalcAngle(Flag::High, Distances[0] - 5) == AnglesHigh[0]);
StaticAssert(CalcAngle(Flag::High, Distances[0]) == AnglesHigh[0]);
StaticAssert(CalcAngle(Flag::High, Distances[2]) == AnglesHigh[2]);
StaticAssert(CalcAngle(Flag::High, Distances[LastDistanceCount]) == AnglesHigh[LastDistanceCount]);
StaticAssert(CalcAngle(Flag::High, Distances[LastDistanceCount] + 100) == AnglesHigh[LastDistanceCount]);
StaticAssert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[2] + Distances[3]) / 2), (AnglesHigh[2] + AnglesHigh[3]) / 2));
StaticAssert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[2] * 3 + Distances[3]) / 4), (AnglesHigh[2] * 3 + AnglesHigh[3]) / 4));

StaticAssert(CalcAngle(Flag::Middle, Distances[0] - 5) == AnglesMedium[0]);
StaticAssert(CalcAngle(Flag::Middle, Distances[0]) == AnglesMedium[0]);
// StaticAssert(CalcAngle(Flag::Middle, Distances[2]) == AnglesMedium[2]);
StaticAssert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]) == AnglesMedium[LastDistanceCount]);
StaticAssert(CalcAngle(Flag::Middle, Distances[LastDistanceCount] + 100) == AnglesMedium[LastDistanceCount]);
StaticAssert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[2] + Distances[3]) / 2), (AnglesMedium[2] + AnglesMedium[3]) / 2));
StaticAssert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[2] * 3 + Distances[3]) / 4), (AnglesMedium[2] * 3 + AnglesMedium[3]) / 4));

// Need to figure out initial position
Shooter::Shooter()
    : m_preloadSensor(shooterPreloadPoterntiometer),
      m_angleSensor(anglePotPort),
      m_ballPresenceSensorUp(ballPresenceSensorUp),
      m_ballPresenceSensorDown(ballPresenceSensorDown),
      m_ballPresenceSensorDown2(ballPresenceSensorDown2)
{
    motor_set_brake_mode(angleMotorPort, motor_brake_mode_e::E_MOTOR_BRAKE_HOLD);
    motor_set_encoder_units(angleMotorPort, motor_encoder_units_e::E_MOTOR_ENCODER_COUNTS);    
    motor_set_reversed(angleMotorPort, true);
    ResetState();

    m_initialAdjustment = true;
    motor_tare_position(angleMotorPort);
    m_lastPos = 0;
}

void Shooter::ResetState()
{
    m_distanceInches = Distances[0];
    // ReportStatus("Shooter angle: %d\n", m_angleSensor.get_value());
}

unsigned int Shooter::CalcAngle()
{
    return ::CalcAngle(m_flag, m_distanceInches);
}

bool MoveToTopFlagPosition()
{
    return joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP);
}

bool MoveToMiddleFlagPosition()
{
    return joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_RIGHT);
}

void Shooter::StopShooting()
{
    if (m_overrideShooting)
    {
        // ReportStatus("Stop shooting\n");
    }
    m_overrideShooting = false; // this is signal to autonomous!
}

bool Shooter::IsShooting()
{
    if (!GetMain().vision.OnTarget())
    {
        // Allow "shoot" button to be pressed all the time and shoot once angle is settled
        if (m_fMoving)
        {
            Assert(!m_overrideShooting); // atonomous should always wait till angle is settled.
            return false;
        }

        if (!m_overrideShooting && !joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
            return false;
    }
    
    // if we do not hava a ball - cancel shooting
    // check for lack of ball - that speeds up the process of shooting when ball is getting in.
    if (BallStatus() == BallPresence::HasBall)
        return true;

    // no ball, cancel shooting.
    StopShooting();
    return false;
}

bool Shooter::IsMovingAngle()
{
    return m_fMoving;
}

void Shooter::InitialAdjustment()
{
    m_count++;
    motor_move(angleMotorPort, -40);
    if (m_count <= 30)
        return;
    int curr = motor_get_position(angleMotorPort);
    // ReportStatus("initial adjustment: %d %d %d\n", curr, m_lastPos, m_count);
    if (m_lastPos == curr && m_count < 200)
        m_count = 200;
    m_lastPos = curr;

    if (m_count == 210) 
    {
        m_initialAdjustment = false;
        motor_tare_position(angleMotorPort);
        m_lastPos = 0;
        StartMoving();
    }
}

static constexpr unsigned int AnglePoints[] = { 15, 31, 150, 700, UINT_MAX};
static constexpr unsigned int AngleSpeeds[] = { 0,  40,  50, 250, 301};

void Shooter::KeepMoving()
{
    int power = 0;
    int current = motor_get_position(angleMotorPort);

    int error = (int) m_angleToMove - current;
    int sign = Sign(error);
    int actualSpeed = motor_get_actual_velocity(angleMotorPort);
    int idealSpeed = SpeedFromDistances(error, AnglePoints, AngleSpeeds);

    m_count++;

    if (m_fMoving)
    {
        // For some reason motor reports rather high speed for many ticks even though reading does not change
        bool slow = abs(actualSpeed) <= 12 || m_lastPos == current;
        if (m_count >= 100 || (idealSpeed == 0 && slow) || ((m_angleToMove - m_angleMovingFrom) * sign < 0 && abs(error) > 20))
        {
            if (abs(error) >= 20)
                ReportStatus("   Angle: error=%d\n", error);
            StopMoving();
            return;
        }

        int diff = idealSpeed - actualSpeed;

        if (idealSpeed != 0)
            power = sign * 30 + idealSpeed * (1 + abs(idealSpeed) / 60) / 5 + diff / 5;
        else
            power = Sign(actualSpeed) * 5;
    }
    else if (abs(error) > 20 || m_adjusting != 0)
    {
        if (m_adjusting == 0)
            m_adjusting = error;
        if (error * m_adjusting > 0)
        {
            power = (22 + m_integral / 2) * sign;
            m_integral++;
            if (m_integral > 30)
                m_integral = 0;
        }
        else
        {
            m_adjusting = 0;
        }
    }
    else
    {
        m_integral = 0;
    }
    
    const int angleMotorSpeed = 75;

    if (power > angleMotorSpeed)
        power = angleMotorSpeed;
    else if (power < -angleMotorSpeed)
        power = -angleMotorSpeed;

    if (PrintDiagnostics(Diagnostics::Angle))
    {
        if (m_fMoving)
            ReportStatus("ANG: (%d) P=%d Dest=%d R=%d, Dist: %d, speed=%d, ideal = %d\n", m_count, power, m_angleToMove, current, current - m_angleToMove,
            actualSpeed, idealSpeed);
        else if (power != 0)
            ReportStatus("ANG ADJ: (%d) P=%d Dest=%d R=%d, Dist: %d\n", m_count, power, m_angleToMove, current, current - m_angleToMove);
    }

    if (m_fMoving && m_count == 1)
        m_power = power;
    else
        m_power = (power + m_power) / 2;

    m_lastPos = current;
    motor_move(angleMotorPort, m_power);
}

void Shooter::StartMoving()
{
    m_fMoving = true;
    m_angleToMove = ConvertAngleToPotentiometer(CalcAngle());
    m_angleMovingFrom = motor_get_position(angleMotorPort);
    m_lastPos = m_angleMovingFrom;
    m_count = 0;
    m_integral = 0;
    m_adjusting = 0;

    // ReportStatus("Angle start moving: %d -> %d\n", m_angleSensor.get_value(), m_angleToMove);
}

void Shooter::StopMoving()
{
    // ReportStatus("Angle: stopped moving: count = %d\n", m_count);
    motor_move(angleMotorPort, 0);
    m_count = 0;
    // m_power = 0;
    m_fMoving = false;
}

void Shooter::SetDistance(unsigned int distance)
{
    // ReportStatus("Shooter::SetDistance(%d)\n", distance);
    m_distanceInches = distance;
    StartMoving();
}

void Shooter::SetFlag(Flag flag)
{
    // ReportStatus("Shooter::SetFlag(%d)\n", (int)flag);
    m_flag = flag;
    StartMoving();
}

// positive is up
void Shooter::MoveAngleRelative(int pos)
{
    StartMoving();
    m_angleToMove = motor_get_position(angleMotorPort) + pos;
}

int Shooter::MovingRelativeTo()
{
    int diff = m_angleToMove - motor_get_position(angleMotorPort);
    if (!m_fMoving && abs(diff) <= 15)
        return 0;
    return diff;
}

void Shooter::UpdateDistanceControls()
{
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A))
        SetDistance(Distances[0]);
    /*
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))    
        SetDistance(Distances[1]);
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_Y))
        SetDistance(Distances[2]);
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
        SetDistance(Distances[3]);
    */
}

void Shooter::OverrideSetShooterMode(bool on)
{
    Assert(isAuto());
    // ReportStatus("start shooting\n");
    m_overrideShooting = on;
}

BallPresence Shooter::BallStatus()
{
    return ::BallStatus(m_ballPresenceSensorUp);
}

BallPresence Shooter::Ball2Status()
{
    /*
    auto status1 = ::BallStatus(m_ballPresenceSensorDown);
    auto status2 = ::BallStatus(m_ballPresenceSensorDown2);
    return BallPresence(max(int(status1), int(status2)));
    */
    unsigned int darkness1 = m_ballPresenceSensorDown.get_value();
    unsigned int darkness2 = m_ballPresenceSensorDown2.get_value();

    if ((darkness1 + darkness2) == 0)
        return BallPresence::NoBall;

    if ((darkness1 + darkness2) > 4100){
        return BallPresence::NoBall;
    }
    if ((darkness1 + darkness2) < 3900){
        return BallPresence::HasBall;
    }

    return BallPresence::Unknown;
}

BallPresence BallStatus(pros::ADIAnalogIn& sensor)
{
    // Check if we can detect ball present.
    // Use two stops to make sure we do not move angle up and down if we are somewhere in gray area (on the boundary)
    unsigned int darkness = sensor.get_value();
    StaticAssert(lightSensorBallIn < lightSensorBallOut);
    bool ballIn = (darkness < lightSensorBallIn);
    bool ballOut = (darkness > lightSensorBallOut);

    if (ballOut){
        return BallPresence::NoBall;
    }
    if (ballIn){
        return BallPresence::HasBall;
    }
    return BallPresence::Unknown;
}

// Preloadign/shooting phases:
// 1) Preloaded, stopped:
//      m_preloadAfterShotCounter = 0
//      m_preloading = false
//      Potentiometer: between ShooterPreloadStart & ShooterPreloadEnd
// 2) Shooting:
//      Same, potentiometer can jump from 0 to 4000
// 3) Shot happened - preloading:
//      Same, but lost ball.
//      m_preloadAfterShotCounter = 150, counting down.
//      m_overrideShooting = false (reset)
// 4) Preloading:
//      Potentiometer goes from 0 to 4000
//      m_preloadAfterShotCounter = 0
//      m_preloading = true
// 5) Preloading - continuation:
//      m_preloadAfterShotCounter = 0
//      m_preloading = true
// 6) Preloading - continuation:
//      Potentiometer is between ShooterPreloadStart & ShooterPreloadEnd
// 7) Preloading - stopping:
//      Potentiometer reaches ShooterPreloadEnd
//      Motor off, potentiometer reading can jump back, but needs to stay between ShooterPreloadStart & ShooterPreloadEnd (otherwise we go to #6)
//      m_preloading = false;
void Shooter::Update()
{
    UpdateDistanceControls();

    const bool userShooting = IsShooting();
    int shooterPreloadPos = m_preloadSensor.get_value();

    m_timeSinseShooting++;
    if (userShooting)
    {
        m_timeSinseShooting = 0;
        UpdateIntakeFromShooter(IntakeShoterEvent::Shooting);
    }

    // Did we go from zero to 4000 on potentiometer?
    // That means shot was done, and we are starting "normal" preload.
    if ((ShooterPreloadStart < shooterPreloadPos && shooterPreloadPos < 3400))
        m_preloadAfterShotCounter = 0;

    if (m_preloadAfterShotCounter > 0)
        m_preloadAfterShotCounter--;

    // ReportStatus("%d %d %d %d\n", m_preloadAfterShotCounter, shooterPreloadPos, (m_preloading), userShooting);

    // remove jiggering by having two stops
    // Start moving if we are below first stop (ShooterPreloadStart)
    // and keep motors moving to further stop (ShooterPreloadEnd)
    // Physically system will move back until it catches next teath on stopper,
    // so we need to have some gap in between those stops not to have ssytem continuosly moving up and down.
    bool needPreload = !m_disablePreload && (m_preloadAfterShotCounter > 0 ||
                                             (m_preloading && shooterPreloadPos > ShooterPreloadEnd) ||
                                             shooterPreloadPos > ShooterPreloadStart);

    if (PrintDiagnostics(Diagnostics::Angle) && needPreload != m_preloading)
        ReportStatus("Preload state: %d\n", needPreload);

    // safety net - if something goes wrong, user needs to have a way to disable preload.
    // This is done by clicking and releasing shooter button quickly while not in the shooting zone.
    if (needPreload && userShooting && !m_userShooting && m_preloadAfterShotCounter == 0)
    {
        if (PrintDiagnostics(Diagnostics::Angle))
            ReportStatus("Disabling preload\n");
        m_disablePreload = true;
        needPreload = false;
    }

    // Check if we can detect ball present.
    BallPresence ball = BallStatus();
    BallPresence ball2 = Ball2Status();
    
    if (ball == BallPresence::HasBall && ballCount == 0)
    {
        ballCount = 1;
    }

    if (ball == BallPresence::HasBall && ball2 == BallPresence::HasBall && (!m_haveBall || !m_haveBall2))
    {
        UpdateIntakeFromShooter(IntakeShoterEvent::TooManyBalls);
        ballCount = 2;
    }

    if (ball != BallPresence::HasBall && m_haveBall)
    {
        ReportStatus("Lost ball\n");
        UpdateIntakeFromShooter(IntakeShoterEvent::LostBall);
        
        ballCount--;

        // Start moving to another flag.
        // Use fuzzy logic here, as auto-aiming might have focused on a different
        int midFlag =  ConvertAngleToPotentiometer(::CalcAngle(Flag::Middle, m_distanceInches));
        int highFlag =  ConvertAngleToPotentiometer(::CalcAngle(Flag::High, m_distanceInches));
        int curr = motor_get_position(angleMotorPort);
        // ReportStatus("Lost ball: curr=%d high=%d low =%d\n", curr, highFlag, midFlag);
        if (abs(midFlag - curr) > abs(highFlag - curr))
            SetFlag(Flag::Middle);
        else
            SetFlag(Flag::High);

        // Did shot just hapened?
        // We can't use potentiometer here, because we are very close to dead zone, so it can jump from 0 to 4000.
        // But having user pressing shoot button and losing the ball is a good indicator we are done.
        // This does not handle case when ball escapes, but shooter still did not fire - we will misfire and reload -
        // likely in time for next ball to land in shooter, so it's not a big issue.
        if (m_timeSinseShooting <= 20)
        {
            m_disablePreload = false;
            StopShooting();
            m_preloadAfterShotCounter = 50;
        }
    }

    m_haveBall = (ball == BallPresence::HasBall);
    m_haveBall2 = (ball2 == BallPresence::HasBall);
    m_userShooting = userShooting;
    m_preloading = needPreload;

    // Check angle direction based on user actions.
    bool topFlag = MoveToTopFlagPosition();
    bool middleFlag = MoveToMiddleFlagPosition();

    if (topFlag)
        SetFlag(Flag::High);
    if (middleFlag)
        SetFlag(Flag::Middle);

    bool shooting = userShooting || needPreload || m_preloadAfterShotCounter > 0;

    // Shooter motor.
    if (shooting)
    {
        motor_move(shooterPort, -shooterMotorSpeed);
    }
    else
    {
        motor_move(shooterPort, 0);
    }

    // Keep moving angle to rigth position
    if (m_initialAdjustment)
        InitialAdjustment();
    else
        KeepMoving();
}
